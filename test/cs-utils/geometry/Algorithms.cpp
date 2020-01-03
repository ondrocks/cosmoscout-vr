////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../src/cs-utils/doctest.hpp"
#include "../../../src/cs-utils/geometry/Algortithms.hpp"
#include "../../../src/cs-utils/geometry/Ray.hpp"
#include "../../../src/cs-utils/geometry/Sphere.hpp"

namespace cs::utils::geom {

static const Sphere     UNIT_SPHERE(glm::dvec3(0.0), 1.0);
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

TEST_CASE("cs::utils::geom::intersection(TLineSegment2, TLineSegment2)") {
  {
    const DLineSegment2 l1{{-1.0, 0.0}, {1.0, 0.0}};
    const DLineSegment2 l2{{0.0, -1.0}, {0.0, 1.0}};

    std::optional<glm::dvec2> p = intersection(l1, l2);
    CHECK_UNARY(p);
    CHECK_EQ(p->x, doctest::Approx(0.0));
    CHECK_EQ(p->y, doctest::Approx(0.0));
  }

  {
    const DLineSegment2 l1{{0.0, 0.0}, {1.0, 1.0}};
    const DLineSegment2 l2{{0.0, 1.0}, {1.0, 0.0}};

    std::optional<glm::dvec2> p = intersection(l1, l2);
    CHECK_UNARY(p);
    CHECK_EQ(p->x, doctest::Approx(0.5));
    CHECK_EQ(p->y, doctest::Approx(0.5));
  }

  {
    const DLineSegment2 l1{{0.0, 0.0}, {1.0, 0.99}};
    const DLineSegment2 l2{{0.0, 1.0}, {1.1, 1.0}};

    std::optional<glm::dvec2> p = intersection(l1, l2);
    CHECK_UNARY_FALSE(p);
  }
}

TEST_CASE("cs::utils::geom::centerOfGravityConvex(Quadrilateral)") {
  {
    glm::dvec2 a{0.0, 0.0};
    glm::dvec2 b{0.0, 1.0};
    glm::dvec2 c{1.0, 1.0};
    glm::dvec2 d{1.0, 0.0};

    DQuadrilateral q{a, b, c, d};
    glm::dvec2     center = centerOfGravityConvex(q);
    CHECK_EQ(center.x, doctest::Approx(0.5));
    CHECK_EQ(center.y, doctest::Approx(0.5));
  }

  {
    glm::dvec2 a{3.5, 0.5};
    glm::dvec2 b{1.0, 0.5};
    glm::dvec2 c{0.5, 2.0};
    glm::dvec2 d{4.0, 1.5};

    DQuadrilateral q{a, b, c, d};
    glm::dvec2     center = centerOfGravityConvex(q);
    CHECK_EQ(center.x, doctest::Approx(13.0 / 6.0));
    CHECK_EQ(center.y, doctest::Approx(7.0 / 6.0));
  }
}

} // namespace cs::utils::geom