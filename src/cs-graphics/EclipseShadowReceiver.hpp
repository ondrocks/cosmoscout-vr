////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ECLIPSE_SHADOW_RECEIVER_HPP
#define CS_GRAPHICS_ECLIPSE_SHADOW_RECEIVER_HPP

#include "../cs-core/GraphicsEngine.hpp"
#include "../cs-core/SolarSystem.hpp"
#include "eclipse-shadows/EclipseShadowCaster.hpp"
#include <VistaOGLExt/VistaGLSLShader.h>
#include <memory>
#include <vector>

namespace cs::scene {
class CelestialObject;

class CelestialBody;
} // namespace cs::scene

namespace cs::graphics {

enum struct CS_GRAPHICS_EXPORT EclipseCalcType : int {
  /// No eclipse will be rendered.
  OFF = 0,

  /// Uses a pre calculated texture to look up the shadow values.
  CARTESIAN_TEXTURE_LOOKUP = 1,

  ANGULAR_TEXTURE_LOOKUP = 2,

  /// Approximate with circles instead of spheres. It is probably the
  /// fastest and most precise method.
  CIRCLE_APPROXIMATION = 3,

  /// The fastest, eclipse renderer. It is the most inaccurate.
  /// http://developer.amd.com/wordpress/media/2012/10/Oat-AmbientApetureLighting.pdf
  AMD_APPROXIMIATION = 4,
};

class CS_GRAPHICS_EXPORT EclipseShadowReceiver {
 public:
  EclipseShadowReceiver(scene::CelestialObject const* shadowReceiver,
      std::shared_ptr<core::GraphicsEngine>           graphicsEngine,
      std::shared_ptr<core::SolarSystem>              solarSystem);

  std::string applyExtensionToShader(std::string_view fragmentShaderSource);

  void initUniforms(VistaGLSLShader const& shader);

  void setupRender(VistaGLSLShader& shader, EclipseCalcType eclipseCalcType, int textureOffset);

  void cleanUpRender(EclipseCalcType eclipseCalcType, int textureOffset);

 private:
  scene::CelestialObject const* const mShadowReceiver;
  const scene::CelestialObject*       mSun = nullptr;

  int mUSunPositionLocation;
  int mUSunRadiusLocation;

  int mUEclipseCalcTypeLocation;

  int mUOccludingBodiesLocation;
  int mUNumOccludingBodiesLocation;

  static constexpr size_t MAX_BODIES = 16;

  std::array<int, MAX_BODIES> mUShadowTextures{};
  std::array<int, MAX_BODIES> mUShadowLength{};
  std::array<int, MAX_BODIES> mUBodyShadowNormals{};

  std::array<EclipseShadowCaster*, MAX_BODIES> mEclipseShadows{};
  int                                          mNumBodies = 0;

  std::shared_ptr<core::GraphicsEngine> mGraphicsEngine;
  std::shared_ptr<core::SolarSystem>    mSolarSystem;
};
} // namespace cs::graphics
#endif // CS_GRAPHICS_ECLIPSE_SHADOW_RECEIVER_HPP
