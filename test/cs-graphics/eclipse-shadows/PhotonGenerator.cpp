////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../src/cs-graphics/eclipse-shadows/PhotonGenerator.hpp"
#include "../../../src/cs-utils/doctest.hpp"

namespace cs::graphics {

const BodyWithAtmosphere EARTH{9.81, 6200000, {149598023000.0}, {0.0, 0.0, 42000.0, {}, {}}};

/// This test is only here for catching errors that produce unreasonable results.
/// Small errors can't be catched by this.
TEST_CASE("cs::graphics::PhotonGenerator Intensity") {
  PhotonGenerator pg{};

  auto   photons = pg.generatePhotons(100'000, EARTH);
  double sum     = 0.0;
  for (const auto& photon : photons) {
    sum += photon.intensity;
  }
  double average = sum / photons.size();

  WARN_EQ(average, doctest::Approx(1.0).epsilon(0.01));
}

/// This test is only here for catching errors that produce unreasonable results.
/// Small errors can't be catched by this.
TEST_CASE("cs::graphics::PhotonGenerator Wavelengths") {
  PhotonGenerator pg{};

  auto     photons = pg.generatePhotons(100'000, EARTH);
  uint64_t min     = std::numeric_limits<uint64_t>::max();
  uint64_t max     = 0ul;

  uint64_t sum = 0ul;

  for (const auto& photon : photons) {
    sum += photon.wavelength;

    if (photon.wavelength < min)
      min = photon.wavelength;

    if (photon.wavelength > max)
      max = photon.wavelength;
  }

  WARN_EQ(min, MIN_WAVELENGTH);
  WARN_EQ(max, MAX_WAVELENGTH);

  double average         = static_cast<double>(sum) / photons.size();
  double expectedAverage = (MIN_WAVELENGTH + MAX_WAVELENGTH) / 2.0;

  WARN_EQ(average, doctest::Approx(expectedAverage).epsilon(0.01));
}
} // namespace cs::graphics