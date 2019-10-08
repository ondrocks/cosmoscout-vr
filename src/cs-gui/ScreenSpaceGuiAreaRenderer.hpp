////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GUI_SCREE_SPACE_GUI_AREA_RENDERER_HPP
#define CS_GUI_SCREE_SPACE_GUI_AREA_RENDERER_HPP

#include "cs_gui_export.hpp"

#include <VistaAspects/VistaObserver.h>
#include <VistaKernel/DisplayManager/VistaViewport.h>
#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>

#include <GL/glew.h>
#include <memory>
#include <vector>

class VistaGLSLShader;

namespace cs::gui {

class ScreenSpaceGuiArea;

class CS_GUI_EXPORT ScreenSpaceGuiAreaRenderer : public IVistaOpenGLDraw {
 public:
  explicit ScreenSpaceGuiAreaRenderer();
  ~ScreenSpaceGuiAreaRenderer() override = default;

  /// Draws the UI to screen.
  bool Do() override;
  bool GetBoundingBox(VistaBoundingBox& bb) override;

  void setGuiAreas(std::vector<ScreenSpaceGuiArea*> const& guiAreas);
  void addGuiArea(ScreenSpaceGuiArea* guiArea);
  void removeGuiArea(ScreenSpaceGuiArea* guiArea);
 private:
  std::unique_ptr<VistaGLSLShader> mShader;

  struct {
    GLint position;
    GLint scale;
    GLint texture;
  } mUniforms{};

  std::vector<ScreenSpaceGuiArea*> mGuiAreas;
};

} // namespace cs::gui

#endif // CS_GUI_SCREE_SPACE_GUI_AREA_RENDERER_HPP
