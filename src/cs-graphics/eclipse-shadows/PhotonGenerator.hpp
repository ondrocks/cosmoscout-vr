////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PHOTON_GENERATOR_HPP
#define CS_GRAPHICS_PHOTON_GENERATOR_HPP

#include "BodyProperties.hpp"
#include "EclipseConstants.hpp"
#include "Photon.hpp"

#include <random>
#include <vector>

namespace cs::graphics {
class PhotonGenerator {
 public:
  PhotonGenerator()  = default;
  ~PhotonGenerator() = default;

  std::vector<Photon> generatePhotons(uint32_t count, BodyWithAtmosphere const& body);

 private:
  std::mt19937_64                         mRNG{std::random_device()()};
  std::uniform_int_distribution<uint32_t> mDistributionWavelength{
      std::uniform_int_distribution<uint32_t>(MIN_WAVELENGTH, MAX_WAVELENGTH)};
};
} // namespace cs::graphics

#endif // CS_GRAPHICS_PHOTON_GENERATOR_HPP
