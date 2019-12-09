#version 430

#extension GL_ARB_compute_variable_group_size: enable

struct Planet {
    double atmosphericHeight;// m
    double gravitationAcceleration;// m/s^2
    double molarMass;// kg / mol
};

struct AtmosphericLayer {
    double baseHeight;// m
    double baseTemperature;// K
    double temperatureLapseRate;// K / m
    double baseDensity;// kg/m^3
};

struct SellmeierCoefficients {
    double a;
    uint numTerms;
    dvec2 terms[8];
};

const uint MIN_WAVELENGTH = 390;
const uint MAX_WAVELENGTH = 749;
const uint NUM_WAVELENGTHS = MAX_WAVELENGTH - MIN_WAVELENGTH + 1;

const double DX = 1.0LF;// m
const double IDEAL_UNIVERSAL_GAS_CONSTANT = 8.31446261815324LF;// J / (mol * K)

uniform Planet planet;
uniform SellmeierCoefficients sellmeierCoefficients;

layout(std430, binding = 2) buffer AtmosphericLayers {
    AtmosphericLayer[] atmosphericLayers;
};

layout(std430, binding = 0) buffer RefractiveIndices {
    double[][NUM_WAVELENGTHS] refractiveIndicesAtAltitudes;// DX steps
};

layout(std430, binding = 1) buffer Densities {
    double[] densitiesAtAltitudes;
};

layout (local_size_variable) in;

AtmosphericLayer layerAtAltitude(double altitude) {
    for (uint i = 1; i < atmosphericLayers.length(); ++i) {
        AtmosphericLayer layer = atmosphericLayers[i];

        if (altitude < layer.baseHeight)
        return atmosphericLayers[i - 1];
    }

    return atmosphericLayers[atmosphericLayers.length() - 1];

    /*if (altitude < 11000.0) {
        return AtmosphericLayer(0.0, 288.15, -0.0065, 1.2250);
    } else if (altitude < 20000.0) {
        return AtmosphericLayer(11000.0, 216.65, 0.0, 0.36391);
    } else if (altitude < 32000.0) {
        return AtmosphericLayer(20000.0, 216.65, 0.001, 0.08803);
    } else {
        return AtmosphericLayer(32000.0, 228.65, 0.0028, 0.01322);
    }*/
}

const double EPSILON = 0.00000001;

double pow(double base, int power) {
    double res = 1.0;// Initialize result

    while (power > 0.0) {
        // If power is odd, multiply x with result
        if (power % 2 == 1) {
            res *= base;
        }

        // n must be even now
        power /= 2;
        base *= base;// Change x to x^2
    }

    return res;
}

double pow(double base, double power) {
    bool negative = power < 0.0LF;
    if (negative) {
        power *= -1.0LF;
    }

    double fraction = power - int(power);
    int integer = int(power - fraction);

    double intPow = pow(base, integer);

    double low = 0.0LF;
    double high = 1.0LF;

    double sqr = sqrt(base);
    double acc = sqr;
    double mid = high / 2.0LF;

    while (abs(mid - fraction) > EPSILON) {
        sqr = sqrt(sqr);

        if (mid <= fraction) {
            low = mid;
            acc *= sqr;
        } else {
            high = mid;
            acc *= (1.0LF / sqr);
        }

        mid = (low + high) / 2.0LF;
    }

    double result = intPow * acc;
    if (negative) {
        return 1.0LF / result;
    }

    return result;
}

const double E = 2.7182818284590452353602874LF;

double exp(double power) { return pow(E, power); }

double densityAtAltitude(double altitude) {
    AtmosphericLayer layer = layerAtAltitude(altitude);

    if (layer.temperatureLapseRate != 0.0LF) {
        double divisor = fma(layer.temperatureLapseRate, altitude - layer.baseHeight, layer.baseTemperature);

        double exponent = 1.0LF + (planet.gravitationAcceleration * planet.molarMass)
        / (IDEAL_UNIVERSAL_GAS_CONSTANT * layer.temperatureLapseRate);
        return layer.baseDensity * pow(layer.baseTemperature / divisor, exponent);
    } else {
        return layer.baseDensity * exp((-planet.gravitationAcceleration * planet.molarMass
        * (altitude - layer.baseHeight)) / (IDEAL_UNIVERSAL_GAS_CONSTANT * layer.baseTemperature));
    }
}

double refractiveIndexAtSeaLevel(uint wavelength) {
    double wavelengthEN2 = 1.0e6LF / double(wavelength * wavelength);

    double sum = 0.0LF;
    for (int i = 0; i < sellmeierCoefficients.numTerms; ++i) {
        sum += sellmeierCoefficients.terms[i].x / (sellmeierCoefficients.terms[i].y - wavelengthEN2);
    }

    return 1.0 + sellmeierCoefficients.a + sum;
}

double refractiveIndexAtAltitude(double altitude, uint wavelength) {
    double refractiveIndexAtSeaLevel = refractiveIndexAtSeaLevel(wavelength);
    double densityAtAlt = densityAtAltitude(altitude);
    double seaLevelDensity = densityAtAltitude(0.0LF);

    return fma(refractiveIndexAtSeaLevel - 1.0LF, densityAtAlt / seaLevelDensity, 1.0LF);
    //return 1.0 + (refractiveIndexAtSeaLevel - 1.0) * (densityAtAlt / seaLevelDensity);
}

void main() {
    uvec2 gid = gl_GlobalInvocationID.xy;

    double altitude = double(gid.x) * DX;
    if (altitude > planet.atmosphericHeight) return;

    uint wavelength = gid.y + MIN_WAVELENGTH;
    if (wavelength > MAX_WAVELENGTH) return;

    refractiveIndicesAtAltitudes[gid.x][gid.y] = refractiveIndexAtAltitude(altitude, wavelength);

    if (gid.y == 0) {
        densitiesAtAltitudes[gid.x] = densityAtAltitude(altitude);
    }
}