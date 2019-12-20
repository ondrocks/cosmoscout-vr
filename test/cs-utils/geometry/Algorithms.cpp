////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../src/cs-utils/doctest.hpp"
#include "../../../src/cs-utils/geometry/Sphere.hpp"
#include "../../../src/cs-utils/geometry/Ray.hpp"
#include "../../../src/cs-utils/geometry/Algortithms.hpp"

namespace cs::utils::geom {

static const Sphere UNIT_SPHERE(glm::dvec3(0.0), 1.0);
static const glm::dvec3 DIR_X(1.0, 0.0, 0.0);
static const glm::dvec3 DIR_Y(0.0, 1.0, 0.0);
static const glm::dvec3 DIR_Z(0.0, 0.0, 1.0);

TEST_CASE("cs::utils::geom::rayHitSphere") {
  const Ray r1(glm::dvec3(-2.0, 0.0, 0.0), DIR_X);
  CHECK_UNARY(rayHitSphere(r1, UNIT_SPHERE));


  const Ray r2(glm::dvec3(2.0, 0.0, 0.0), -DIR_X);
  CHECK_UNARY(rayHitSphere(r2, UNIT_SPHERE));


  const Ray r3(glm::dvec3(0.0, 2.0, 0.0), -DIR_Y);
  CHECK_UNARY(rayHitSphere(r3, UNIT_SPHERE));

  // TODO more cases!
  WARN(false);
}

} // namespace cs::utils::geom