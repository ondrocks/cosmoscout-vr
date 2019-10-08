////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ScreenSpaceGuiAreaRenderer.hpp"

#include "GuiItem.hpp"
#include "ScreenSpaceGuiArea.hpp"
#include "../cs-utils/FrameTimings.hpp"
#include "../cs-utils/filesystem.hpp"

#include <VistaMath/VistaBoundingBox.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaTexture.h>

namespace cs::gui {
ScreenSpaceGuiAreaRenderer::ScreenSpaceGuiAreaRenderer()
    : mShader(std::make_unique<VistaGLSLShader>()) {

  mShader->InitVertexShaderFromString(cs::utils::filesystem::loadToString(
      "../share/resources/shaders/ScreenSpaceGuiArea.vert.glsl"));
  mShader->InitFragmentShaderFromString(cs::utils::filesystem::loadToString(
      "../share/resources/shaders/ScreenSpaceGuiArea.frag.glsl"));
  mShader->Link();

  mUniforms.position = mShader->GetUniformLocation("iPosition");
  mUniforms.scale = mShader->GetUniformLocation("iScale");
  mUniforms.texture = mShader->GetUniformLocation("iTexture");
}

bool ScreenSpaceGuiAreaRenderer::Do() {
  utils::FrameTimings::ScopedTimer timer("User Interface");

  glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);

  mShader->Bind();

  mShader->SetUniform(mUniforms.texture, 0);

  for (const auto& area : mGuiAreas) {
    // draw back-to-front
    auto const& items = area->getItems();
    for (auto item = items.rbegin(); item != items.rend(); ++item) {
      if ((*item)->getIsEnabled()) {
        float posX = (*item)->getRelPositionX() + (*item)->getRelOffsetX();
        float posY = 1 - (*item)->getRelPositionY() - (*item)->getRelOffsetY();
        mShader->SetUniform(mUniforms.position, posX, posY);

        float scaleX = (*item)->getRelSizeX();
        float scaleY = (*item)->getRelSizeY();
        mShader->SetUniform(mUniforms.scale, scaleX, scaleY);

        (*item)->getTexture()->Bind(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        (*item)->getTexture()->Unbind(GL_TEXTURE0);
      }
    }
  }

  mShader->Release();

  glPopAttrib();

  return true;
}

bool ScreenSpaceGuiAreaRenderer::GetBoundingBox(VistaBoundingBox& bb) {
  float min(std::numeric_limits<float>::min());
  float max(std::numeric_limits<float>::max());
  float fMin[3] = {min, min, min};
  float fMax[3] = {max, max, max};

  bb.SetBounds(fMin, fMax);

  return true;
}

void ScreenSpaceGuiAreaRenderer::setGuiAreas(
    std::vector<ScreenSpaceGuiArea*> const& guiAreas) {
  mGuiAreas = guiAreas;
}

void ScreenSpaceGuiAreaRenderer::addGuiArea(ScreenSpaceGuiArea* guiArea) {
  mGuiAreas.push_back(guiArea);
}

void ScreenSpaceGuiAreaRenderer::removeGuiArea(ScreenSpaceGuiArea* guiArea) {
  std::remove(mGuiAreas.begin(), mGuiAreas.end(), guiArea);
}

} // namespace cs::gui