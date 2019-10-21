////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "WorldSpaceGuiArea.hpp"

#include "../cs-utils/FrameTimings.hpp"
#include "../cs-utils/filesystem.hpp"
#include "GuiItem.hpp"

#include <VistaKernel/DisplayManager/VistaDisplayManager.h>
#include <VistaKernel/DisplayManager/VistaProjection.h>
#include <VistaKernel/DisplayManager/VistaViewport.h>
#include <VistaKernel/VistaSystem.h>
#include <VistaMath/VistaBoundingBox.h>
#include <VistaMath/VistaGeometries.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaTexture.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cs::gui {

std::atomic_size_t                                 WorldSpaceGuiArea::instanceCounter = 0;
std::unique_ptr<internal::WorldSpaceGuiAreaShader> WorldSpaceGuiArea::shader;

////////////////////////////////////////////////////////////////////////////////////////////////////

WorldSpaceGuiArea::WorldSpaceGuiArea(int width, int height)
    : mWidth(width)
    , mHeight(height) {

  if (instanceCounter++ == 0) {
    shader = std::make_unique<internal::WorldSpaceGuiAreaShader>();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

WorldSpaceGuiArea::~WorldSpaceGuiArea() {
  if (--instanceCounter == 0) {
    shader.reset();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void WorldSpaceGuiArea::setWidth(int width) {
  mWidth = width;
  updateItems();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void WorldSpaceGuiArea::setHeight(int height) {
  mHeight = height;
  updateItems();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int WorldSpaceGuiArea::getWidth() const {
  return mWidth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int WorldSpaceGuiArea::getHeight() const {
  return mHeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void WorldSpaceGuiArea::setIgnoreDepth(bool ignore) {
  mIgnoreDepth = ignore;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool WorldSpaceGuiArea::getIgnoreDepth() const {
  return mIgnoreDepth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool WorldSpaceGuiArea::getUseLinearDepthBuffer() const {
  return mUseLinearDepthBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void WorldSpaceGuiArea::setUseLinearDepthBuffer(bool bEnable) {
  mUseLinearDepthBuffer = bEnable;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool WorldSpaceGuiArea::calculateMousePosition(
    VistaVector3D const& vRayOrigin, VistaVector3D const& vRayEnd, int& x, int& y) {

  VistaRay   ray(vRayOrigin, vRayEnd - vRayOrigin);
  VistaPlane plane;

  VistaVector3D intersection;

  if (!plane.CalcIntersection(ray, intersection)) {
    return false;
  }

  x = (int)((intersection[0] + 0.5) * mWidth);
  y = (int)((-intersection[1] + 0.5) * mHeight);

  return intersection[0] >= -0.5f && intersection[0] <= 0.5f && intersection[1] >= -0.5f &&
         intersection[1] <= 0.5f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool WorldSpaceGuiArea::Do() {
  utils::FrameTimings::ScopedTimer timer("User Interface");
  if (mIgnoreDepth)
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  else
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (mIgnoreDepth) {
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
  }

  auto  uniforms      = shader->mUniforms;
  auto& shaderProgram = shader->getProgram();

  shaderProgram.Bind();

  if (mUseLinearDepthBuffer) {
    double near, far;
    GetVistaSystem()
        ->GetDisplayManager()
        ->GetCurrentRenderInfo()
        ->m_pViewport->GetProjection()
        ->GetProjectionProperties()
        ->GetClippingRange(near, far);
    shaderProgram.SetUniform(uniforms.farClip, (float)far);
    shaderProgram.SetUniform(uniforms.useLinearDepth, mUseLinearDepthBuffer);
  }

  // get modelview and projection matrices
  GLfloat glMat[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, &glMat[0]);
  glm::mat4 modelViewMat = glm::make_mat4(glMat);

  glGetFloatv(GL_PROJECTION_MATRIX, &glMat[0]);
  glUniformMatrix4fv(uniforms.matProjection, 1, GL_FALSE, glMat);

  // draw back-to-front
  auto const& items = getItems();
  for (auto item = items.rbegin(); item != items.rend(); ++item) {
    if ((*item)->getIsEnabled()) {
      auto localMat = glm::translate(
          modelViewMat, glm::vec3((*item)->getRelPositionX() + (*item)->getRelOffsetX() - 0.5,
                            -(*item)->getRelPositionY() - (*item)->getRelOffsetY() + 0.5, 0.0));
      localMat =
          glm::scale(localMat, glm::vec3((*item)->getRelSizeX(), (*item)->getRelSizeY(), 1.f));

      (*item)->getTexture()->Bind(GL_TEXTURE0);
      glUniformMatrix4fv(uniforms.matModelView, 1, GL_FALSE, glm::value_ptr(localMat));
      shaderProgram.SetUniform(uniforms.texture, 0);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      (*item)->getTexture()->Unbind(GL_TEXTURE0);
    }
  }

  shaderProgram.Release();

  if (mIgnoreDepth) {
    glDepthMask(GL_TRUE);
  }

  glPopAttrib();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool WorldSpaceGuiArea::GetBoundingBox(VistaBoundingBox& oBoundingBox) {
  float fMin[3] = {-1.f, -1.f, -0.000001f};
  float fMax[3] = {1.f, 1.f, 0.000001f};

  oBoundingBox.SetBounds(fMin, fMax);

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace internal {
WorldSpaceGuiAreaShader::WorldSpaceGuiAreaShader() {
  mShader.InitVertexShaderFromString(cs::utils::filesystem::loadToString(
      "../share/resources/shaders/WorldSpaceGuiArea.vert.glsl"));
  mShader.InitFragmentShaderFromString(cs::utils::filesystem::loadToString(
      "../share/resources/shaders/WorldSpaceGuiArea.frag.glsl"));
  mShader.Link();

  mUniforms.matModelView   = mShader.GetUniformLocation("uMatModelView");
  mUniforms.matProjection  = mShader.GetUniformLocation("uMatProjection");
  mUniforms.useLinearDepth = mShader.GetUniformLocation("uUseLinearDepth");
  mUniforms.texture        = mShader.GetUniformLocation("iTexture");
  mUniforms.farClip        = mShader.GetUniformLocation("iFarClip");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VistaGLSLShader& WorldSpaceGuiAreaShader::getProgram() {
  return mShader;
}

} // namespace internal

} // namespace cs::gui
