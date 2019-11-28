#version 430

#extension GL_ARB_compute_variable_group_size: enable

// TODO make configurable
const uint MIN_WAVELENGTH = 390u;
const uint MAX_WAVELENGTH = 749u;
const uint NUM_WAVELENGTHS = MAX_WAVELENGTH - MIN_WAVELENGTH + 1;

// Size: 24 bytes -> ~40,000,000 photons per available gigabyte of ram
struct Photon {
    vec2 position;// m
    vec2 direction;// normalized
    uint wavelength;// nm
    float intensity;// 0..1 should start at 1
};

// maybe generate in GPU with rng?
layout(std430, binding = 0) buffer Photons {
    Photon photons[];
};

layout(std430, binding = 1) buffer RefractiveIndices {
    float[][NUM_WAVELENGTHS] refractiveIndicesAtAltitudes;// DX steps
};

layout(std430, binding = 2) buffer Densities {
    float[] densitiesAtAltitudes;
};

struct Planet {
    float radius;// m
    float atmosphericHeight;// m
    float seaLevelMolecularNumberDensity;// cm^−3
};

uniform Planet planet;

// TODO make configurable
const float DL = 1000.0;// m
const float DX = 10.0;// m

layout (local_size_variable) in;

float densityAtAltitude(float altitude) {
    return densitiesAtAltitudes[uint(altitude)];
}

float refractiveIndexAtSeaLevel(uint wavelength) {
    return refractiveIndicesAtAltitudes[0][wavelength - MIN_WAVELENGTH];
}

float refractiveIndexAtAltitude(float altitude, uint wavelength) {
    return refractiveIndicesAtAltitudes[uint(altitude)][wavelength - MIN_WAVELENGTH];
}

float partialRefractiveIndex(float altitude, float altitudeDelta, uint wavelength) {
    float refrIndexPlusDelta = refractiveIndexAtAltitude(altitudeDelta, wavelength);
    float refrIndex = refractiveIndexAtAltitude(altitude, wavelength);

    return (refrIndexPlusDelta - refrIndex) / DX;
}

/// Moves the photon to its next location.
void traceRay(inout Photon photon) {
    float altitude = length(photon.position) - planet.radius;
    float altDx = length(photon.position + vec2(DX, 0.0)) - planet.radius;
    float altDy = length(photon.position + vec2(0.0, DX)) - planet.radius;
    float altD1Approx = length(photon.position + DL * photon.direction) - planet.radius;

    if (altitude < planet.atmosphericHeight && altDx < planet.atmosphericHeight && altDy < planet.atmosphericHeight && altD1Approx < planet.atmosphericHeight) {
        float ni = refractiveIndexAtAltitude(altitude, photon.wavelength);
        float pnx = partialRefractiveIndex(altitude, altDx, photon.wavelength);
        float pny = partialRefractiveIndex(altitude, altDy, photon.wavelength);
        vec2 dn = vec2(pnx, pny);

        float ni1 = refractiveIndexAtAltitude(altD1Approx, photon.wavelength);
        vec2 direction = ((ni * photon.direction) + (dn * DL)) / ni1;
        photon.direction = normalize(direction);
    }

    photon.position += (DL * photon.direction);
}

double molecularNumberDensityAtAltitude(float altitude) {
    double seaLevelDensity = double(densityAtAltitude(0.0));
    return double(planet.seaLevelMolecularNumberDensity) * (double(densityAtAltitude(altitude)) / seaLevelDensity);
}

/// I am to lazy to do the squaring every time my self.
double square(double value) {
    return value * value;
}

double rayleighScatteringCrossSection(uint wavelength) {
    // TODO Normally wavelength should be converted with a factor of 1.0e-7, but for no particular reason 2.1e-8 works best.
    //      Let's not talk about this :/
    double wavelengthInCM = double(wavelength) * 2.3e-8LF;
    double wavelengthInCM4 = square(square(wavelengthInCM));

    double refractiveIndex = double(refractiveIndexAtSeaLevel(wavelength));
    double refractiveIndex2 = refractiveIndex * refractiveIndex;

    double molecularNumberDensity = molecularNumberDensityAtAltitude(0.0);
    double molecularNumberDensity2 = square(molecularNumberDensity);

    const double kingCorrectionFactor = 1.05LF;
    const double PI_F = 3.14159265358979323846LF;
    const double PI_F_3 = PI_F * PI_F * PI_F;

    double dividend = 24.0LF * PI_F_3 * square(refractiveIndex2 - 1.0LF);
    double divisor = wavelengthInCM4 * molecularNumberDensity2 * square(refractiveIndex2 + 2.0LF);
    return (dividend / divisor) * kingCorrectionFactor;
}

// TODO maybe precompute in a 2D map?
double rayleighVolumeScatteringCoefficient(float altitude, uint wavelength) {
    double sigma = rayleighScatteringCrossSection(wavelength);
    double mnd = molecularNumberDensityAtAltitude(altitude);
    return mnd * sigma;
}

/// [2, 2] Padé approximant for exp(x).
/// Since there is no exp function for doubles, this is a good and fast approximation for small x.
/// This is enough for this particular application.
double approxE(double x) {
    return (square(x + 3) + 3) / (square(x - 3) + 3);
}

/// Applies rayleigh scattering to the photon for this step.
void attenuateLight(inout Photon photon, vec2 oldPosition) {
    float altitude = length(oldPosition) - planet.radius;

    double beta = rayleighVolumeScatteringCoefficient(altitude, photon.wavelength);

    // TODO don't know what to do with this for now... maybe make it configurable per planet?
    /// This value simulates particles in the upper atmosphere. On earth a value of 1.0e-6 corresponds to an L4 eclipse
    /// and 1.0e-4 produces an L0 eclipse.
    double alpha = 15000.0 < altitude && altitude < 20000.0 ? 2.0e-5 : 0.0;

    photon.intensity = float(double(photon.intensity) * approxE(-(alpha + beta) * DL));
}

/// Does a single step of the ray tracing. It moves the photon to the next location and applies
/// rayleigh scattering to it.
void tracePhoton(inout Photon photon) {
    vec2 oldPosition = vec2(photon.position);

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

    float atmosphereRadius = planet.radius + planet.atmosphericHeight;

    float distFromCenter = length(photon.position);
    while (!exitedAtmosphere && distFromCenter > planet.radius) {
        tracePhoton(photon);
        distFromCenter = length(photon.position);

        if (!enteredAtmosphere && distFromCenter < atmosphereRadius) {
            enteredAtmosphere = true;
        }

        if (enteredAtmosphere && distFromCenter > atmosphereRadius) {
            exitedAtmosphere = true;
        }
    }

    if (!exitedAtmosphere || distFromCenter <= planet.radius) {
        photon.intensity = -1.0;
    }

    photons[passId] = photon;
}