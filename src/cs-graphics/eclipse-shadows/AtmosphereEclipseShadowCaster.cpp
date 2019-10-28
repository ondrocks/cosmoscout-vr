////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AtmosphereEclipseShadowCaster.hpp"
#include "AtmosphereEclipseTextureGenerator.hpp"

namespace cs::graphics {

AtmosphereEclipseShadowCaster::AtmosphereEclipseShadowCaster(
    core::Settings::BodyProperties const& bodyProperties) {
  mRadius = bodyProperties.meanRadius + bodyProperties.atmosphere->height;

  AtmosphereEclipseTextureGenerator textureGenerator{};

  // TODO photon count
  auto [texture, xScalingExponent] =
      textureGenerator.createShadowMap(bodyProperties, 1'000'000);

  mScalingExponent = xScalingExponent;

  glGenTextures(1, &mShadowTexture);
  glBindTexture(GL_TEXTURE_2D, mShadowTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texture.mWidth, texture.mHeight, 0, GL_RGBA, GL_FLOAT,
      texture.dataPtr());
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

AtmosphereEclipseShadowCaster::~AtmosphereEclipseShadowCaster() {
  glDeleteTextures(1, &mShadowTexture);
}

void AtmosphereEclipseShadowCaster::bind(GLenum textureUnit) {
  glBindTexture(textureUnit, mShadowTexture);
}

void AtmosphereEclipseShadowCaster::unbind(GLenum textureUnit) {
  glBindTexture(textureUnit, 0);
}

} // namespace cs::graphics