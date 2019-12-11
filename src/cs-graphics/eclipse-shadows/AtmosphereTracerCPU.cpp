////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AtmosphereTracerCPU.hpp"
#include "../../cs-utils/ThreadPool.hpp"
#include "../../cs-utils/filesystem.hpp"
#include "EclipseConstants.hpp"
#include "LUTPrecalculator.hpp"
#include <GL/glew.h>
#include <iostream>

double square(double value) {
  return value * value;
}

namespace cs::graphics {

void AtmosphereTracerCPU::init() {
  mLutPrecalculator = std::make_unique<LUTPrecalculator>();
}

std::variant<GPUBuffer, CPUBuffer> AtmosphereTracerCPU::traceThroughAtmosphere(
    CPUBuffer& photonBuffer, const BodyWithAtmosphere& body, double xPosition) {

  auto [ssboRefractiveIndices, ssboDensities] = mLutPrecalculator->createLUT(body);

  auto heightDim         = static_cast<uint32_t>(body.atmosphere.height);
  auto refractiveIndexes = std::vector<std::array<double, NUM_WAVELENGTHS>>(heightDim);
  auto densities         = std::vector<double>(heightDim);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboRefractiveIndices);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(double) * heightDim * NUM_WAVELENGTHS,
      refractiveIndexes.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDensities);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(double) * heightDim, densities.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  glDeleteBuffers(1, &ssboRefractiveIndices);
  glDeleteBuffers(1, &ssboDensities);

  const double DL = 1000.0; // m
  const double DX = 10.0;   // m

  glm::dvec3 bodyPosition = glm::dvec3(xPosition, 0.0, 0.0);

  auto densityAtAltitude = [&](double altitude) -> double {
    if (altitude < body.atmosphere.height && altitude >= 0.0) {
      return densities[static_cast<size_t>(altitude)];
    }
    return 0.0;
  };

  auto refractiveIndexAtSeaLevel = [&](uint64_t wavelength) -> double {
    return refractiveIndexes[0][wavelength - MIN_WAVELENGTH];
  };

  auto refractiveIndexAtAltitude = [&](double altitude, uint64_t wavelength) -> double {
    if (altitude < body.atmosphere.height && altitude >= 0.0) {
      return refractiveIndexes[static_cast<uint64_t>(altitude)][wavelength - MIN_WAVELENGTH];
    }
    return 1.0;
  };

  auto partialRefractiveIndex = [&](double altitude, double altitudeDelta,
                                    uint64_t wavelength) -> double {
    double refrIndexPlusDelta = refractiveIndexAtAltitude(altitudeDelta, wavelength);
    double refrIndex          = refractiveIndexAtAltitude(altitude, wavelength);

    return (refrIndexPlusDelta - refrIndex) / DX;
  };

  auto calcAltitude = [&](glm::dvec3 position) -> double {
    return glm::length(position - bodyPosition) - body.meanRadius;
  };

  auto traceRay = [&](Photon& photon) -> void {
    double altitude    = calcAltitude(photon.position);
    double altDx       = calcAltitude(photon.position + glm::dvec3(DX, 0.0, 0.0));
    double altDy       = calcAltitude(photon.position + glm::dvec3(0.0, DX, 0.0));
    double altDz       = calcAltitude(photon.position + glm::dvec3(0.0, 0.0, DX));
    double altD1Approx = calcAltitude(photon.position + DL * photon.direction);

    double     ni  = refractiveIndexAtAltitude(altitude, photon.wavelength);
    double     pnx = partialRefractiveIndex(altitude, altDx, photon.wavelength);
    double     pny = partialRefractiveIndex(altitude, altDy, photon.wavelength);
    double     pnz = partialRefractiveIndex(altitude, altDz, photon.wavelength);
    glm::dvec3 dn  = glm::dvec3(pnx, pny, pnz);

    double     ni1       = refractiveIndexAtAltitude(altD1Approx, photon.wavelength);
    glm::dvec3 direction = ((ni * photon.direction) + (dn * DL)) / ni1;
    photon.direction     = normalize(direction);

    photon.position += (DL * photon.direction);
  };

  auto molecularNumberDensityAtAltitude = [&](double altitude) -> double {
    double seaLevelDensity = densityAtAltitude(0.0);
    return body.atmosphere.seaLevelMolecularNumberDensity *
           (densityAtAltitude(altitude) / seaLevelDensity);
  };

  auto rayleighScatteringCrossSection = [&](uint64_t wavelength) -> double {
    // TODO Normally wavelength should be converted with a factor of 1.0e-7, but for no particular
    // reason 2.1e-8 works best.
    //      Let's not talk about this :/
    double wavelengthInCM  = static_cast<double>(wavelength) * 1.0e-7;
    double wavelengthInCM4 = square(square(wavelengthInCM));

    double refractiveIndex  = refractiveIndexAtSeaLevel(wavelength);
    double refractiveIndex2 = refractiveIndex * refractiveIndex;

    double molecularNumberDensity  = molecularNumberDensityAtAltitude(0.0);
    double molecularNumberDensity2 = molecularNumberDensity * molecularNumberDensity;

    const double kingCorrectionFactor = 1.05;
    const double PI_F                 = 3.14159265358979323846;
    const double PI_F_3               = PI_F * PI_F * PI_F;

    double dividend = 24.0 * PI_F_3 * square(refractiveIndex2 - 1.0);
    double divisor  = wavelengthInCM4 * molecularNumberDensity2 * square(refractiveIndex2 + 2.0);
    return (dividend / divisor) * kingCorrectionFactor;
  };

  auto rayleighVolumeScatteringCoefficient = [&](double altitude, uint64_t wavelength) -> double {
    double sigma = rayleighScatteringCrossSection(wavelength);
    double mnd   = molecularNumberDensityAtAltitude(altitude);
    return mnd * sigma;
  };

  /// Applies rayleigh scattering to the photon for this step.
  auto attenuateLight = [&](Photon& photon, glm::dvec3 oldPosition) -> void {
    double altitude = calcAltitude(oldPosition);

    double beta = rayleighVolumeScatteringCoefficient(altitude, photon.wavelength);

    // TODO don't know what to do with this for now... maybe make it configurable per planet?
    /// This value simulates particles in the upper atmosphere. On earth a value of 1.0e-6
    /// corresponds to an L4 eclipse and 1.0e-4 produces an L0 eclipse.
    double alpha = 15000.0 < altitude && altitude < 20000.0 ? 1.0e-5 : 0.0;

    photon.intensity *= std::exp(-(alpha + beta) * DL);
  };

  auto tracePhoton = [&](Photon& photon) -> void {
    glm::dvec3 oldPosition = photon.position;

    traceRay(photon);
    attenuateLight(photon, oldPosition);
  };

  utils::ThreadPool              tp(std::thread::hardware_concurrency());
  std::vector<std::future<void>> tasks;
  tasks.reserve(photonBuffer.size());

  for (auto&& photon : photonBuffer) {
    tasks.push_back(tp.enqueue([&] {
      bool enteredAtmosphere = false;
      bool exitedAtmosphere  = false;

      double atmosphereRadius = body.meanRadius + body.atmosphere.height;

      double distFromCenter = glm::length(photon.position - bodyPosition);
      size_t counter = 0;
      while (!exitedAtmosphere && distFromCenter > body.meanRadius) {
        tracePhoton(photon);
        distFromCenter = glm::length(photon.position - bodyPosition);

        if (!enteredAtmosphere && distFromCenter < atmosphereRadius) {
          enteredAtmosphere = true;
        }

        if (enteredAtmosphere && distFromCenter > atmosphereRadius) {
          exitedAtmosphere = true;
        }

        // TODO Investigate why some photons do not finish.
        if (counter++ == 1000) {
          //std::cout << photon << std::endl;
          break;
        }
      }

      if (!exitedAtmosphere || distFromCenter <= body.meanRadius) {
        photon.intensity = -1.0;
      }
    }));
  }

  for (auto&& task : tasks) {
    task.get();
  }
  /*
    std::vector<glm::dvec3> positions(photonBuffer.size());
    for (int i = 0; i < photonBuffer.size(); ++i) {
      //std::cout << photonBuffer[i] << std::endl;
      positions[i] = (photonBuffer[i].position / 6371000.0) * 20.0;
    }

    auto objString = utils::verticesToObjString(positions);
    utils::filesystem::saveToFile(objString, "photon_positions_exit.obj");*/

  return photonBuffer;
}
} // namespace cs::graphics