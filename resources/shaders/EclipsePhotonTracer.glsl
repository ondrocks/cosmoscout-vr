#version 430

#extension GL_ARB_compute_variable_group_size: enable

layout (local_size_variable) in;

// TODO make configurable
const uint MIN_WAVELENGTH = 390u;
const uint MAX_WAVELENGTH = 749u;
const uint NUM_WAVELENGTHS = MAX_WAVELENGTH - MIN_WAVELENGTH + 1;

// Size: 24 bytes -> ~40,000,000 photons per available gigabyte of ram
struct Photon {
    dvec3 position;// m                         3 * 8 = 24
    double intensity;// 0..1 should start at 1  1 * 8 = 56
    dvec3 direction;// normalized               3 * 8 = 48
    uint wavelength;// nm                       1 * 4 = 60
};

// maybe generate in GPU with rng?
layout(std430, binding = 0) buffer Photons {
    Photon photons[];
};

layout(std430, binding = 1) buffer RefractiveIndices {
    double[][NUM_WAVELENGTHS] refractiveIndicesAtAltitudes;// DX steps
};

layout(std430, binding = 2) buffer Densities {
    double[] densitiesAtAltitudes;
};

struct Planet {
    double xPosition;
    double radius;// m
    double atmosphericHeight;// m
    double seaLevelMolecularNumberDensity;// cm^−3
};

// TODO make configurable
const double DL = 1000.0LF;// m
const double DX = 10.0LF;// m

uniform Planet planet;

double densityAtAltitude(double altitude) {
    if (altitude < planet.atmosphericHeight && altitude >= 0.0) {
        return densitiesAtAltitudes[uint(altitude)];
    }
    return 0.0;
}

double refractiveIndexAtSeaLevel(uint wavelength) {
    return refractiveIndicesAtAltitudes[0][wavelength - MIN_WAVELENGTH];
}

double refractiveIndexAtAltitude(double altitude, uint wavelength) {
    if (altitude < planet.atmosphericHeight && altitude >= 0.0) {
        return refractiveIndicesAtAltitudes[uint(altitude)][wavelength - MIN_WAVELENGTH];
    }
    return 1.0;
}

double partialRefractiveIndex(double altitude, double altitudeDelta, uint wavelength) {
    double refrIndexPlusDelta = refractiveIndexAtAltitude(altitudeDelta, wavelength);
    double refrIndex = refractiveIndexAtAltitude(altitude, wavelength);

    return (refrIndexPlusDelta - refrIndex) / DX;
}

double calcAltitude(dvec3 position) {
    return length(position - dvec3(planet.xPosition, 0.0LF, 0.0LF)) - planet.radius;
}

/// Moves the photon to its next location.
void traceRay(inout Photon photon) {
    double altitude = calcAltitude(photon.position);
    double altDx = calcAltitude(photon.position + dvec3(DX, 0.0LF, 0.0LF));
    double altDy = calcAltitude(photon.position + dvec3(0.0LF, DX, 0.0LF));
    double altDz = calcAltitude(photon.position + dvec3(0.0LF, 0.0LF, DX));
    double altD1Approx = calcAltitude(photon.position + DL * photon.direction);

    double ni = refractiveIndexAtAltitude(altitude, photon.wavelength);
    double pnx = partialRefractiveIndex(altitude, altDx, photon.wavelength);
    double pny = partialRefractiveIndex(altitude, altDy, photon.wavelength);
    double pnz = partialRefractiveIndex(altitude, altDz, photon.wavelength);
    dvec3 dn = dvec3(pnx, pny, pnz);

    double ni1 = refractiveIndexAtAltitude(altD1Approx, photon.wavelength);
    dvec3 direction = ((ni * photon.direction) + (dn * DL)) / ni1;
    photon.direction = normalize(direction);

    photon.position += (DL * photon.direction);
}

double molecularNumberDensityAtAltitude(double altitude) {
    double seaLevelDensity = densityAtAltitude(0.0LF);
    return planet.seaLevelMolecularNumberDensity * (densityAtAltitude(altitude) / seaLevelDensity);
}

/// I am to lazy to do the squaring every time my self.
double square(double value) {
    return value * value;
}

double rayleighScatteringCrossSection(uint wavelength) {
    const double NM_TO_CM = 1.0e-7LF;

    double wavelengthInCM = double(wavelength) * NM_TO_CM;
    double wavelengthInCM4 = square(square(wavelengthInCM));

    double refractiveIndex = refractiveIndexAtSeaLevel(wavelength);
    double refractiveIndex2 = refractiveIndex * refractiveIndex;

    double molecularNumberDensity = molecularNumberDensityAtAltitude(0.0);
    double molecularNumberDensity2 = square(molecularNumberDensity);

    const double kingCorrectionFactor = 1.05LF;
    const double PI_D = 3.14159265358979323846LF;
    const double PI_D_3 = PI_D * PI_D * PI_D;

    double dividend = 24.0LF * PI_D_3 * square(refractiveIndex2 - 1.0LF);
    double divisor = wavelengthInCM4 * molecularNumberDensity2 * square(refractiveIndex2 + 2.0LF);
    return (dividend / divisor) * kingCorrectionFactor;
}

// TODO maybe precompute in a 2D map?
double rayleighVolumeScatteringCoefficient(double altitude, uint wavelength) {
    double sigma = rayleighScatteringCrossSection(wavelength);
    double mnd = molecularNumberDensityAtAltitude(altitude);

    const double CM_TO_M = 1.0e2LF; // cm^-1 -> m^-1
    return mnd * sigma * CM_TO_M;
}

/// [2, 2] Padé approximant for exp(x).
/// Since there is no exp function for doubles, this is a good and fast approximation for small x.
/// This is enough for this particular application.
double approxExp(double x) {
    return (square(x + 3.0LF) + 3.0LF) / (square(x - 3.0LF) + 3.0LF);
}

/// Applies rayleigh scattering to the photon for this step.
void attenuateLight(inout Photon photon, dvec3 oldPosition) {
    double altitude = calcAltitude(oldPosition);

    double beta = rayleighVolumeScatteringCoefficient(altitude, photon.wavelength);

    // TODO don't know what to do with this for now... maybe make it configurable per planet?
    /// This value simulates particles in the upper atmosphere. On earth a value of 1.0e-6 corresponds to an L4 eclipse
    /// and 1.0e-4 produces an L0 eclipse.
    double alpha = 15000.0LF < altitude && altitude < 20000.0LF ? 1.0e-5LF : 0.0LF;

    photon.intensity *= approxExp(-(alpha + beta) * DL);
}

/// Does a single step of the ray tracing. It moves the photon to the next location and applies
/// rayleigh scattering to it.
void tracePhoton(inout Photon photon) {
    dvec3 oldPosition = dvec3(photon.position);

    traceRay(photon);
    attenuateLight(photon, oldPosition);
}

uniform uint pass;
uniform uint passSize;

void main() {
    uint gid = gl_GlobalInvocationID.x;
    uint passId = (pass * passSize) + gid;
    if (passId >= photons.length()) return;

    Photon photon = photons[passId];

    bool enteredAtmosphere = false;
    bool exitedAtmosphere = false;

    double atmosphereRadius = planet.radius + planet.atmosphericHeight;

    double distFromCenter = length(photon.position - dvec3(planet.xPosition, 0.0LF, 0.0LF));
    uint counter = 0;
    while (!exitedAtmosphere && distFromCenter > planet.radius) {
        tracePhoton(photon);
        distFromCenter = length(photon.position - dvec3(planet.xPosition, 0.0LF, 0.0LF));

        if (!enteredAtmosphere && distFromCenter < atmosphereRadius) {
            enteredAtmosphere = true;
        }

        if (enteredAtmosphere && distFromCenter > atmosphereRadius) {
            exitedAtmosphere = true;
        }

        // Safety abort condition to avoid infinite loops on sketchy data.
        if (counter++ == 100000) {
            photon.intensity = -2.0;
            break;
        }
    }

    if (!exitedAtmosphere || distFromCenter <= planet.radius) {
        photon.intensity = -1.0LF;
    }

    photons[passId] = photon;
}