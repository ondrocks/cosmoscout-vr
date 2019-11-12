////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ECLIPSE_SHADOW_RECEIVER_HPP
#define CS_GRAPHICS_ECLIPSE_SHADOW_RECEIVER_HPP

#include "eclipse-shadows/EclipseShadowCaster.hpp"
#include <VistaOGLExt/VistaGLSLShader.h>
#include <memory>
#include <vector>
namespace cs::scene {
class CelestialObject;

class CelestialBody;
} // namespace cs::scene

namespace cs::graphics {
enum struct EclipseCalcType;

class SimpleEclipseShadowCaster;

class EclipseShaderExtension {
 public:
  explicit EclipseShaderExtension(cs::scene::CelestialObject const* shadowReceiver);

  void init(cs::scene::CelestialObject const*                        sun,
      std::vector<std::shared_ptr<const cs::scene::CelestialObject>> shadowCasters);

  std::string applyExtensionToShader(std::string_view fragmentShaderSource);

  void initUniforms(VistaGLSLShader const& shader);

  void setupRender(VistaGLSLShader& shader, EclipseCalcType eclipseCalcType, int textureOffset);

  void cleanUpRender(EclipseCalcType eclipseCalcType, int textureOffset);

 private:
  std::vector<std::shared_ptr<const cs::scene::CelestialObject>> mShadowObjects;
  cs::scene::CelestialObject const*                              mSun;
  cs::scene::CelestialObject const* const                        mShadowReceiver;

  int mUSunPositionLocation;
  int mUSunRadiusLocation;

  int mUEclipseCalcTypeLocation;

  int mUOccludingBodiesLocation;
  int mUNumOccludingBodiesLocation;

  static constexpr size_t MAX_BODIES = 16;

  std::array<int, MAX_BODIES> mUShadowTextures;
  std::array<int, MAX_BODIES> mUScalingFactor;
  std::array<int, MAX_BODIES> mUShadowLength;
  std::array<int, MAX_BODIES> mUBodyShadowNormals;

  std::array<EclipseShadowCaster*, MAX_BODIES> eclipseShadows{};
  int                                          mNumBodies = 0;
};
} // namespace cs::graphics
#endif // CS_GRAPHICS_ECLIPSE_SHADOW_RECEIVER_HPP
