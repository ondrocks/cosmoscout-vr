////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_CORE_TOOLS_MARK_HPP
#define CS_CORE_TOOLS_MARK_HPP

#include "Tool.hpp"

#include <VistaBase/VistaColor.h>
#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaVertexArrayObject.h>

#include <atomic>
#include <glm/glm.hpp>
#include <memory>

namespace cs::scene {
class CelestialAnchorNode;
} // namespace cs::scene

class VistaBufferObject;
class VistaOpenGLNode;
class VistaVertexArrayObject;


namespace cs::core {
class TimeControl;
class SolarSystem;
class InputManager;
class GuiManager;
class GraphicsEngine;

namespace tools {

namespace internal {
class MarkShader;
}

/// A mark is a single point on the surface. It is selectable and draggable.
class CS_CORE_EXPORT Mark : public IVistaOpenGLDraw, public Tool {
 public:
  /// Observable properties to get updates on state changes.
  cs::utils::Property<glm::dvec2> pLngLat   = glm::dvec2(0.0);
  cs::utils::Property<bool>       pHovered  = false;
  cs::utils::Property<bool>       pSelected = false;
  cs::utils::Property<bool>       pActive   = false;
  cs::utils::Property<glm::vec3>  pColor    = glm::vec3(0.75, 0.75, 1.0);

  Mark(std::shared_ptr<InputManager> pInputManager, std::shared_ptr<SolarSystem> pSolarSystem,
      std::shared_ptr<GraphicsEngine> graphicsEngine, std::shared_ptr<GuiManager> pGuiManager,
      std::shared_ptr<TimeControl> pTimeControl, std::string const& sCenter,
      std::string const& sFrame);

  Mark(Mark const& other);

  virtual ~Mark();

  std::shared_ptr<cs::scene::CelestialAnchorNode> const& getAnchor() const;
  std::shared_ptr<cs::scene::CelestialAnchorNode>&       getAnchor();

  /// Called from Tools class.
  void update() override;

  /// Inherited from IVistaOpenGLDraw.
  bool Do() override;
  bool GetBoundingBox(VistaBoundingBox& bb) override;

 protected:
  std::shared_ptr<InputManager>   mInputManager;
  std::shared_ptr<SolarSystem>    mSolarSystem;
  std::shared_ptr<GraphicsEngine> mGraphicsEngine;
  std::shared_ptr<GuiManager>     mGuiManager;
  std::shared_ptr<TimeControl>    mTimeControl;

  std::shared_ptr<cs::scene::CelestialAnchorNode> mAnchor = nullptr;
  VistaOpenGLNode*                                mParent = nullptr;

  double mOriginalDistance = -1.0;

 private:
  void initData(std::string const& sCenter, std::string const& sFrame);

  static std::atomic_size_t instanceCounter;
  static std::unique_ptr<internal::MarkShader> shader;

  int mSelfLngLatConnection = -1, mHoveredNodeConnection = -1, mSelectedNodeConnection = -1,
      mButtonsConnection = -1, mHoveredPlanetConnection = -1, mHeightScaleConnection = -1;
};

namespace internal {
class MarkShader {
 public:
  MarkShader();
  virtual ~MarkShader() = default;

  VistaGLSLShader& getProgram();

  void bind();
  void release();

  struct {
    int32_t matModelView;
    int32_t matProjection;
    int32_t hoverSelectActive;
    int32_t farClip;
    int32_t color;
  } mUniforms{};

  static constexpr size_t INDEX_COUNT = 36;

 private:
  VistaGLSLShader        mShader{};
  VistaVertexArrayObject mVAO;
  VistaBufferObject      mVBO;
  VistaBufferObject      mIBO;

  static const std::string SHADER_VERT;
  static const std::string SHADER_FRAG;
};
} // namespace internal

} // namespace tools
} // namespace cs::core

#endif // CS_CORE_TOOLS_MARK_HPP
