////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PhotonGenerator.hpp"
#include "../../cs-utils/geometry/Algortithms.hpp"
#include "../../cs-utils/geometry/Ray.hpp"
#include "../../cs-utils/geometry/Sphere.hpp"
#include "../../cs-utils/parallel.hpp"
#include "BlackBodySpectrum.hpp"
#include "EclipseConstants.hpp"
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <mutex>
#include <random>

namespace cs::graphics {

std::vector<Photon> PhotonGenerator::generatePhotons(
    uint32_t count, BodyWithAtmosphere const& body) {
  // 1. calculate target area
  double rOcc  = body.meanRadius + body.atmosphere.height;
  double rOcc2 = rOcc * rOcc;

  double r2 = (rOcc + SUN_RADIUS) * (rOcc + SUN_RADIUS);

  double d  = body.orbit.semiMajorAxisSun;
  double d2 = d * d;

  double a    = (rOcc2 * std::sqrt(d2 - r2) * d) / (-rOcc * r2 + d2 * rOcc) - body.meanRadius;
  double xOcc = (rOcc * (SUN_RADIUS + rOcc)) / d;

  utils::geom::DSphere sphereBody({xOcc, 0.0, 0.0}, body.meanRadius);
  utils::geom::DSphere sphereAtmosphere({xOcc, 0.0, 0.0}, body.meanRadius + body.atmosphere.height);

  double yLower = body.meanRadius;
  double yUpper = body.meanRadius + a;

  std::uniform_real_distribution<double> targetDistribution(yLower, yUpper);

  std::vector<Photon> photons;
  photons.reserve(count);

  std::mutex lock{};

  // 2. for each i in count
  cs::utils::executeParallel(count, [&](size_t i) {
    // 3. sample random point in target area,
    glm::dvec3 target(xOcc, targetDistribution(mRNG), 0.0);

    // 4. sample random point on suns surface
    double     xSun = xOcc - d;
    glm::dvec3 sunCenter(-xSun, 0.0, 0.0);
    double     angularRadSun = std::asin(SUN_RADIUS / glm::length(target - sunCenter));

    std::uniform_real_distribution<double> angleRng(-angularRadSun, angularRadSun);
    glm::dvec3                             randYawPitch{};
    do {
      randYawPitch = glm::dvec3(0.0, angleRng(mRNG), angleRng(mRNG));
    } while (glm::length(randYawPitch) > angularRadSun);

    glm::dvec3 aimingVector = target - sunCenter;
    glm::dquat rotation(randYawPitch);
    aimingVector         = rotation * aimingVector;
    glm::dvec3 direction = -glm::normalize(aimingVector);
    glm::dvec3 origin    = target + aimingVector;

    utils::geom::DRay3 photonRay(origin, direction);

    // 5. validate resulting ray to ensure it can pass through atmosphere
    if (utils::geom::rayHitSphere(photonRay, sphereBody) ||
        !utils::geom::rayHitSphere(photonRay, sphereAtmosphere)) {
      return;
    }

    // 6. calculate limb darkening for start point
    double limbDarkening = calculateLimbDarkening(glm::length(randYawPitch) / angularRadSun);

    // 7. get random wavelength in visible spectrum
    uint32_t wavelength = mDistributionWavelength(mRNG);
    double   intensity  = INTENSITY_LUT[wavelength - MIN_WAVELENGTH];

    // 8. from wavelength and limb darkening get an intensity
    double intensityAdjusted = limbDarkening * intensity;

    // 9. shoot photon on to atmosphere
    double distanceToAtmosphere = utils::geom::raySphereDistance(photonRay,
        utils::geom::DSphere(sphereBody.center, body.meanRadius + body.atmosphere.height));

    glm::dvec3 startPosition = origin + direction * (distanceToAtmosphere + 10.0);

    if (glm::length(startPosition - sphereBody.center) <= sphereAtmosphere.radius) {
      std::lock_guard<std::mutex> guard(lock);
      photons.emplace_back(startPosition, direction, intensityAdjusted, wavelength);
    }
  });

  std::cout << "Number of Photons send: " << photons.size() << std::endl;

  return photons;
}
} // namespace cs::graphics