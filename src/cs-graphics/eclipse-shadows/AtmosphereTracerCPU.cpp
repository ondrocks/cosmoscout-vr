////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AtmosphereTracerCPU.hpp"
#include "../../cs-utils/ThreadPool.hpp"
#include "../../cs-utils/parallel.hpp"
#include "EclipseConstants.hpp"
#include "LUTPrecalculator.hpp"
#include <GL/glew.h>
#include <glm/ext/scalar_constants.hpp>
#include <iostream>

double square(double value) {
  return value * value;
}

namespace cs::graphics {

void AtmosphereTracerCPU::init() {
  mLutPrecalculator = std::make_unique<LUTPrecalculator>();
}

const double DL = 1000.0; // m
const double DX = 10.0;   // m

std::variant<GPUBuffer, CPUBuffer> AtmosphereTracerCPU::traceThroughAtmosphere(
    CPUBuffer& photonBuffer, const BodyWithAtmosphere& body, double xPosition) {

  auto [ssboRefractiveIndices, ssboDensities] = mLutPrecalculator->createLUT(body);

  auto heightDim         = static_cast<uint32_t>(body.atmosphere.height);
  auto refractiveIndexes = detail::RefractiveIndexLUT(heightDim);
  auto densities         = std::vector<double>(heightDim);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboRefractiveIndices);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(double) * heightDim * NUM_WAVELENGTHS,
      refractiveIndexes.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDensities);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(double) * heightDim, densities.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  glDeleteBuffers(1, &ssboRefractiveIndices);
  glDeleteBuffers(1, &ssboDensities);

  glm::dvec3           bodyPosition = glm::dvec3(xPosition, 0.0, 0.0);
  detail::PhotonTracer photonTracer(body, bodyPosition, refractiveIndexes, densities);

  cs::utils::executeParallel(photonBuffer.size(), [&](size_t i) {
    auto& photon            = photonBuffer[i];
    bool  enteredAtmosphere = false;
    bool  exitedAtmosphere  = false;

    double atmosphereRadius = body.meanRadius + body.atmosphere.height;

    double distFromCenter = glm::length(photon.position - bodyPosition);
    size_t counter        = 0;
    auto   photonStart    = photon;

    while (!exitedAtmosphere && distFromCenter > body.meanRadius) {
      glm::dvec3 oldPosition = photon.position;

      photonTracer.traceRay(photon);
      photonTracer.attenuateLight(photon, oldPosition);

      distFromCenter = glm::length(photon.position - bodyPosition);

      if (!enteredAtmosphere && distFromCenter < atmosphereRadius) {
        enteredAtmosphere = true;
      }

      if (enteredAtmosphere && distFromCenter > atmosphereRadius) {
        exitedAtmosphere = true;
      }

      // This is only for debug and error purposes. It protects against unforeseen infinite loops
      // and logs the photons properties.
      if (counter++ == 100'000) {
        std::cerr << "Photon did loop forever!\n"
                  << "        Photon: " << photon << "\n"
                  << "      Altitude: "
                  << glm::distance(photon.position, bodyPosition) - body.meanRadius << "\n"
                  << "  Start Photon: " << photonStart << "\n"
                  << "Start Altitude: "
                  << glm::distance(photonStart.position, bodyPosition) - body.meanRadius
                  << std::endl;
        photon.intensity = -1.0;
        break;
      }
    }

    if (!exitedAtmosphere || distFromCenter <= body.meanRadius) {
      photon.intensity = -1.0;
    }
  });

  return photonBuffer;
}

namespace detail {

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::calcAltitude(glm::dvec3 const& position) noexcept {
  return glm::distance(position, mBodyPosition) - mBody.meanRadius;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::densityAtAltitude(double altitude) noexcept {
  if (altitude < mBody.atmosphere.height && altitude >= 0.0) {
    return mDensities.at(static_cast<size_t>(altitude));
  }
  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::refractiveIndexAtSeaLevel(uint64_t wavelength) noexcept {
  return mRefractiveIndexes[0].at(wavelength - MIN_WAVELENGTH);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::refractiveIndexAtAltitude(double altitude, uint64_t wavelength) noexcept {
  if (altitude < mBody.atmosphere.height && altitude >= 0.0) {
    return mRefractiveIndexes.at(static_cast<uint64_t>(altitude)).at(wavelength - MIN_WAVELENGTH);
  }
  return 1.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::partialRefractiveIndex(
    double altitude, double altitudeDelta, uint64_t wavelength) noexcept {
  double refrIndexPlusDelta = refractiveIndexAtAltitude(altitudeDelta, wavelength);
  double refrIndex          = refractiveIndexAtAltitude(altitude, wavelength);

  return (refrIndexPlusDelta - refrIndex) / DX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PhotonTracer::traceRay(Photon& photon) noexcept {
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::molecularNumberDensityAtAltitude(double altitude) noexcept {
  double seaLevelDensity = densityAtAltitude(0.0);
  return mBody.atmosphere.seaLevelMolecularNumberDensity *
         (densityAtAltitude(altitude) / seaLevelDensity);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::rayleighScatteringCrossSection(uint64_t wavelength) noexcept {
  double const NM_TO_CM = 1.0e-7;

  double wavelengthInCM  = static_cast<double>(wavelength) * NM_TO_CM;
  double wavelengthInCM4 = square(square(wavelengthInCM));

  double refractiveIndex  = refractiveIndexAtSeaLevel(wavelength);
  double refractiveIndex2 = refractiveIndex * refractiveIndex;

  double molecularNumberDensity  = molecularNumberDensityAtAltitude(0.0);
  double molecularNumberDensity2 = molecularNumberDensity * molecularNumberDensity;

  const double kingCorrectionFactor = 1.05;
  const double PI_3                 = glm::pi<double>() * glm::pi<double>() * glm::pi<double>();

  double dividend = 24.0 * PI_3 * square(refractiveIndex2 - 1.0);
  double divisor  = wavelengthInCM4 * molecularNumberDensity2 * square(refractiveIndex2 + 2.0);
  return (dividend / divisor) * kingCorrectionFactor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double PhotonTracer::rayleighVolumeScatteringCoefficient(
    double altitude, uint64_t wavelength) noexcept {
  double const sigma   = rayleighScatteringCrossSection(wavelength);
  double const mnd     = molecularNumberDensityAtAltitude(altitude);
  double const CM_TO_M = 1.0e2;
  return mnd * sigma * CM_TO_M;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PhotonTracer::attenuateLight(Photon& photon, glm::dvec3 const& oldPosition) noexcept {
  double altitude = calcAltitude(oldPosition);
  double beta     = rayleighVolumeScatteringCoefficient(altitude, photon.wavelength);

  // TODO don't know what to do with this for now... maybe make it configurable per planet?
  /// This value simulates particles in the upper atmosphere. On earth a value of 1.0e-6
  /// corresponds to an L4 eclipse and 1.0e-4 produces an L0 eclipse.
  double alpha = 15000.0 < altitude && altitude < 20000.0 ? 1.0e-6 : 0.0;

  photon.intensity *= std::exp(-(alpha + beta) * DL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PhotonTracer::PhotonTracer(BodyWithAtmosphere const& body, glm::dvec3 const& bodyPosition,
    RefractiveIndexLUT const& refractiveIndexes, std::vector<double> const& densities)
    : mBody(body)
    , mBodyPosition(bodyPosition)
    , mRefractiveIndexes(refractiveIndexes)
    , mDensities(densities) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace detail
} // namespace cs::graphics