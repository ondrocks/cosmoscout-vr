////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_GENERATOR_HPP
#define CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_GENERATOR_HPP

#include "../../cs-utils/SimpleTexture.hpp"
#include "BodyProperties.hpp"
#include "ColorConverter.hpp"
#include "Photon.hpp"
#include "AtmosphereTracer.hpp"
#include "TextureTracer.hpp"
#include <random>
#include <memory>

namespace cs::graphics {
class AtmosphereEclipseTextureGenerator {
 public:
  AtmosphereEclipseTextureGenerator();

  cs::utils::Texture4f createShadowMap(BodyWithAtmosphere const& body, size_t photonCount);

 private:
  std::vector<Photon> generatePhotons(uint32_t count, BodyWithAtmosphere const& body);

  std::mt19937_64                         mRNG;
  std::uniform_int_distribution<uint32_t> mDistributionWavelength;
  std::bernoulli_distribution             mDistributionBoolean;

  std::unique_ptr<AtmosphereTracer> mAtmosphereTracer;
  std::unique_ptr<TextureTracer>    mTextureTracer;
  ColorConverter                    mColorConverter;
};
} // namespace cs::graphics

#endif // CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_GENERATOR_HPP
