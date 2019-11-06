#version 430

#extension GL_ARB_compute_variable_group_size: enable

layout (local_size_variable) in;

struct Planet {
    float atmosphericHeight;              // m
    float gravitationAcceleration;        // m/s^2
    float molarMass;                      // kg / mol
    float seaLevelMolecularNumberDensity; // cm^−3
};

struct AtmosphericLayer {
    float baseHeight;           // m
    float baseTemperature;      // K
    float temperatureLapseRate; // K / m
    float baseDensity;          // kg/m^3
};

struct SellmeierCoefficients {
    float a;
    uint numTerms;
    vec2 terms[8];
};

const uint MIN_WAVELENGTH = 390;
const uint MAX_WAVELENGTH = 749;
const uint NUM_WAVELENGTHS = MAX_WAVELENGTH - MIN_WAVELENGTH + 1;

const float DX = 1.0;                               // m
const float IDEAL_UNIVERSAL_GAS_CONSTANT = 8.31447; // J / (mol * K)

uniform Planet planet;
uniform SellmeierCoefficients sellmeierCoefficients;

layout(std430, binding = 2) buffer AtmosphericLayers {
    AtmosphericLayer[] atmosphericLayers;
};

layout(std430, binding = 0) buffer RefractiveIndices {
    float[][NUM_WAVELENGTHS] refractiveIndicesAtAltitudes; // DX steps
};

layout(std430, binding = 1) buffer Densities {
    float[] densitiesAtAltitudes;
};

AtmosphericLayer layerAtAltitude(float altitude) {
    /*for (uint i = 1; i < atmosphericLayers.length(); ++i) {
        AtmosphericLayer layer = atmosphericLayers[i];

        if (altitude < layer.baseHeight)
            return atmosphericLayers[i - 1];
    }

    return atmosphericLayers[atmosphericLayers.length() - 1];
*/
    if (altitude < 11000.0) {
        return AtmosphericLayer(288.15, -0.0065, 1.2250, 0.0);
    } else if (altitude < 20000.0) {
        return AtmosphericLayer(216.65, 0.0, 0.36391, 11000.0);
    } else if (altitude < 32000.0) {
        return AtmosphericLayer(216.65, 0.001, 0.08803, 20000.0);
    } else {
        return AtmosphericLayer(228.65, 0.0028, 0.01322, 32000.0);
    }
}

float densityAtAltitude(float altitude) {
    AtmosphericLayer layer = layerAtAltitude(altitude);

    if (layer.temperatureLapseRate != 0.0) {
        float divisor = fma(layer.temperatureLapseRate, altitude - layer.baseHeight, layer.baseTemperature);

        float exponent = 1.0 + (planet.gravitationAcceleration * planet.molarMass)
        / (IDEAL_UNIVERSAL_GAS_CONSTANT * layer.temperatureLapseRate);
        return layer.baseDensity * pow(layer.baseTemperature / divisor, exponent);
    } else {
        return layer.baseDensity * exp((-planet.gravitationAcceleration * planet.molarMass
        * (altitude - layer.baseHeight)) / (IDEAL_UNIVERSAL_GAS_CONSTANT * layer.baseTemperature));
    }
}

float refractiveIndexAtSeaLevel(uint wavelength) {
    float wavelengthEN2 = 1.0e6 / float(wavelength * wavelength);

    float sum = 0.0;
    for (int i = 0; i < sellmeierCoefficients.numTerms; ++i) {
        sum += sellmeierCoefficients.terms[i].x / (sellmeierCoefficients.terms[i].y - wavelengthEN2);
    }

    return 1.0 + sellmeierCoefficients.a + sum;
}

float refractiveIndexAtAltitude(float altitude, uint wavelength) {
    float refractiveIndexAtSeaLevel = refractiveIndexAtSeaLevel(wavelength);
    float densityAtAlt = densityAtAltitude(altitude);
    float seaLevelDensity = densityAtAltitude(0.0);

    return fma(refractiveIndexAtSeaLevel - 1.0, densityAtAlt / seaLevelDensity, 1.0);
    //return 1.0 + (refractiveIndexAtSeaLevel - 1.0) * (densityAtAlt / seaLevelDensity);
}

void main() {
    uvec2 gid = gl_GlobalInvocationID.xy;

    float altitude = float(gid.x) * DX;
    if(altitude > planet.atmosphericHeight) return;

    uint wavelength = gid.y + MIN_WAVELENGTH;
    if(wavelength > MAX_WAVELENGTH) return;

    refractiveIndicesAtAltitudes[gid.x][gid.y] = refractiveIndexAtAltitude(altitude, wavelength);

    if(gid.y == 0) {
        densitiesAtAltitudes[gid.x] = densityAtAltitude(altitude);
    }
}