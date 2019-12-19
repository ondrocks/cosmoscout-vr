////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../src/cs-graphics/eclipse-shadows/BlackBodySpectrum.hpp"
#include "../../../src/cs-utils/doctest.hpp"
#include <glm/gtx/norm.hpp>

namespace cs::graphics {
TEST_CASE("cs::graphics::INTENSITY_LUT") {
  double sum = 0;

  for (const auto& item : INTENSITY_LUT) {
    sum += item;
  }

  double average = sum / INTENSITY_LUT.size();
  CHECK_EQ(average, doctest::Approx(1.0));
}

TEST_CASE("cs::graphics::calculateLimbDarkening") {
  const int64_t STEPS     = 2000;
  const double  STEP_SIZE = 2.0 / STEPS;

  double   sum     = 0.0;
  uint64_t counter = 0;
  for (int x = -STEPS; x <= STEPS; ++x) {
    for (int y = -STEPS; y <= STEPS; ++y) {
      glm::dvec2 p(x * STEP_SIZE, y * STEP_SIZE);
      double     l = glm::length2(p);
      if (l <= 1.0) {
        sum += calculateLimbDarkening(std::sqrt(l));
        counter++;
      }
    }
  }

  double average = sum / counter;
  CHECK_EQ(average, doctest::Approx(1.0));
}
} // namespace cs::graphics