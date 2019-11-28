////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_CASTER_HPP
#define CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_CASTER_HPP

#include "BodyProperties.hpp"
#include "EclipseShadowCaster.hpp"

namespace cs::graphics {
class CS_GRAPHICS_EXPORT AtmosphereEclipseShadowCaster : public EclipseShadowCaster {
 public:
  explicit AtmosphereEclipseShadowCaster(BodyWithAtmosphere const& body);
  virtual ~AtmosphereEclipseShadowCaster();

  void bind(GLenum textureUnit) override;
  void unbind(GLenum textureUnit) override;

 private:
  uint32_t mShadowTexture;
};
} // namespace cs::graphics

#endif // CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_CASTER_HPP
