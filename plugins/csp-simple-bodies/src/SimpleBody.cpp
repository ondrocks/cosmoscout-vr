////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleBody.hpp"

#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-graphics/TextureLoader.hpp"
#include "../../../src/cs-utils/FrameTimings.hpp"
#include "../../../src/cs-utils/utils.hpp"

#include <VistaKernel/GraphicsManager/VistaSceneGraph.h>
#include <VistaKernel/GraphicsManager/VistaTransformNode.h>
#include <VistaKernel/VistaSystem.h>
#include <VistaKernelOpenSGExt/VistaOpenSGMaterialTools.h>
#include <VistaMath/VistaBoundingBox.h>
#include <VistaOGLExt/VistaOGLUtils.h>

#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace csp::simplebodies {

////////////////////////////////////////////////////////////////////////////////////////////////////

const uint32_t GRID_RESOLUTION_X = 200;
const uint32_t GRID_RESOLUTION_Y = 100;

////////////////////////////////////////////////////////////////////////////////////////////////////

const char* SimpleBody::SPHERE_VERT = R"(
uniform vec3 uSunDirection;
uniform vec3 uRadii;
uniform mat4 uMatModelView;
uniform mat4 uMatProjection;

// inputs
layout(location = 0) in vec2 iGridPos;

// outputs
out vec2 vTexCoords;
out vec3 vPosition;
out vec3 vCenter;
out vec2 vLonLat;

const float PI = 3.141592654;

void main()
{
    vTexCoords = vec2(iGridPos.x, 1-iGridPos.y);
    vLonLat.x = iGridPos.x * 2.0 * PI;
    vLonLat.y = (iGridPos.y-0.5) * PI;
    vPosition = uRadii * vec3(
        -sin(vLonLat.x) * cos(vLonLat.y),
        -cos(vLonLat.y+PI*0.5),
        -cos(vLonLat.x) * cos(vLonLat.y)
    );
    vPosition   = (uMatModelView * vec4(vPosition, 1.0)).xyz;
    vCenter     = (uMatModelView * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    gl_Position =  uMatProjection * vec4(vPosition, 1);

    if (gl_Position.w > 0) {
      gl_Position /= gl_Position.w;
      if (gl_Position.z >= 1) {
        gl_Position.z = 0.999999;
      }
    }
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const char* SimpleBody::SPHERE_FRAG = R"(
uniform vec3 uSunDirection;
uniform sampler2D uSurfaceTexture;
uniform float uAmbientBrightness;
uniform float uSunIlluminance;
uniform float uFarClip;

// inputs
in vec2 vTexCoords;
in vec3 vSunDirection;
in vec3 vPosition;
in vec3 vCenter;
in vec2 vLonLat;

// outputs
layout(location = 0) out vec3 oColor;

vec3 SRGBtoLINEAR(vec3 srgbIn)
{
  vec3 bLess = step(vec3(0.04045),srgbIn);
  return mix( srgbIn/vec3(12.92), pow((srgbIn+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
}
    
void main()
{
    oColor = texture(uSurfaceTexture, vTexCoords).rgb;

    #ifdef ENABLE_HDR
      oColor = SRGBtoLINEAR(oColor);
    #endif

    oColor = oColor * uSunIlluminance;

    #ifdef ENABLE_LIGHTING
      vec3 normal = normalize(vPosition - vCenter);
      float light = max(dot(normal, uSunDirection), 0.0);
      oColor = mix(oColor*uAmbientBrightness, oColor, light);
    #endif

    gl_FragDepth = length(vPosition) / uFarClip;
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleBody::SimpleBody(std::shared_ptr<cs::core::Settings> settings,
    std::shared_ptr<cs::core::SolarSystem> solarSystem, std::string const& sCenterName,
    std::string const& sFrameName, double tStartExistence, double tEndExistence)
    : cs::scene::CelestialBody(sCenterName, sFrameName, tStartExistence, tEndExistence)
    , mSettings(std::move(settings))
    , mSolarSystem(std::move(solarSystem))
    , mRadii(cs::core::SolarSystem::getRadii(sCenterName)) {
  pVisibleRadius = mRadii[0];

  // For rendering the sphere, we create a 2D-grid which is warped into a sphere in the vertex
  // shader. The vertex positions are directly used as texture coordinates.
  std::vector<float>    vertices(GRID_RESOLUTION_X * GRID_RESOLUTION_Y * 2);
  std::vector<unsigned> indices((GRID_RESOLUTION_X - 1) * (2 + 2 * GRID_RESOLUTION_Y));

  for (uint32_t x = 0; x < GRID_RESOLUTION_X; ++x) {
    for (uint32_t y = 0; y < GRID_RESOLUTION_Y; ++y) {
      vertices[(x * GRID_RESOLUTION_Y + y) * 2 + 0] = 1.F / (GRID_RESOLUTION_X - 1) * x;
      vertices[(x * GRID_RESOLUTION_Y + y) * 2 + 1] = 1.F / (GRID_RESOLUTION_Y - 1) * y;
    }
  }

  uint32_t index = 0;

  for (uint32_t x = 0; x < GRID_RESOLUTION_X - 1; ++x) {
    indices[index++] = x * GRID_RESOLUTION_Y;
    for (uint32_t y = 0; y < GRID_RESOLUTION_Y; ++y) {
      indices[index++] = x * GRID_RESOLUTION_Y + y;
      indices[index++] = (x + 1) * GRID_RESOLUTION_Y + y;
    }
    indices[index] = indices[index - 1];
    ++index;
  }

  mSphereVAO.Bind();

  mSphereVBO.Bind(GL_ARRAY_BUFFER);
  mSphereVBO.BufferData(vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  mSphereIBO.Bind(GL_ELEMENT_ARRAY_BUFFER);
  mSphereIBO.BufferData(indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

  mSphereVAO.EnableAttributeArray(0);
  mSphereVAO.SpecifyAttributeArrayFloat(
      0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0, &mSphereVBO);

  mSphereVAO.Release();
  mSphereIBO.Release();
  mSphereVBO.Release();

  // Recreate the shader if lighting or HDR rendering mode are toggled.
  mEnableLightingConnection = mSettings->mGraphics.pEnableLighting.connect(
      [this](bool /*enabled*/) { mShaderDirty = true; });
  mEnableHDRConnection =
      mSettings->mGraphics.pEnableHDR.connect([this](bool /*enabled*/) { mShaderDirty = true; });

  // Add to scenegraph.
  VistaSceneGraph* pSG = GetVistaSystem()->GetGraphicsManager()->GetSceneGraph();
  mGLNode.reset(pSG->NewOpenGLNode(pSG->GetRoot(), this));
  VistaOpenSGMaterialTools::SetSortKeyOnSubtree(
      mGLNode.get(), static_cast<int>(cs::utils::DrawOrder::ePlanets));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleBody::~SimpleBody() {
  mSettings->mGraphics.pEnableLighting.disconnect(mEnableLightingConnection);
  mSettings->mGraphics.pEnableHDR.disconnect(mEnableHDRConnection);

  VistaSceneGraph* pSG = GetVistaSystem()->GetGraphicsManager()->GetSceneGraph();
  pSG->GetRoot()->DisconnectChild(mGLNode.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SimpleBody::configure(Plugin::Settings::SimpleBody const& settings) {
  if (mSimpleBodySettings.mTexture != settings.mTexture) {
    mTexture = cs::graphics::TextureLoader::loadFromFile(settings.mTexture);
  }
  mSimpleBodySettings = settings;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SimpleBody::setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun) {
  mSun = sun;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleBody::getIntersection(
    glm::dvec3 const& rayOrigin, glm::dvec3 const& rayDir, glm::dvec3& pos) const {

  glm::dmat4 transform = glm::inverse(getWorldTransform());

  // Transform ray into planet coordinate system.
  glm::dvec4 origin(rayOrigin, 1.0);
  origin = transform * origin;

  glm::dvec4 direction(rayDir, 0.0);
  direction = transform * direction;
  direction = glm::normalize(direction);

  double b    = glm::dot(origin, direction);
  double c    = glm::dot(origin, origin) - mRadii[0] * mRadii[0];
  double fDet = b * b - c;

  if (fDet < 0.0) {
    return false;
  }

  fDet = std::sqrt(fDet);
  pos  = (origin + direction * (-b - fDet));

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double SimpleBody::getHeight(glm::dvec2 /*lngLat*/) const {
  // This is why we call them 'SimpleBodies'.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::dvec3 SimpleBody::getRadii() const {
  return mRadii;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleBody::Do() {
  if (!getIsInExistence() || !pVisible.get()) {
    return true;
  }

  cs::utils::FrameTimings::ScopedTimer timer("Simple Bodies");

  if (mShaderDirty) {
    mShader = VistaGLSLShader();

    // (Re-)create sphere shader.
    std::string defines = "#version 330\n";

    if (mSettings->mGraphics.pEnableHDR.get()) {
      defines += "#define ENABLE_HDR\n";
    }

    if (mSettings->mGraphics.pEnableLighting.get()) {
      defines += "#define ENABLE_LIGHTING\n";
    }

    mShader.InitVertexShaderFromString(defines + SPHERE_VERT);
    mShader.InitFragmentShaderFromString(defines + SPHERE_FRAG);
    mShader.Link();

    mShaderDirty = false;
  }

  mShader.Bind();

  glm::vec3 sunDirection(1, 0, 0);
  float     sunIlluminance(1.F);
  float     ambientBrightness(mSettings->mGraphics.pAmbientBrightness.get());

  if (getCenterName() == "Sun") {
    // If the SimpleBody is actually the sun, we have to calculate the lighting differently.
    if (mSettings->mGraphics.pEnableHDR.get()) {
      double sceneScale = 1.0 / mSolarSystem->getObserver().getAnchorScale();
      sunIlluminance    = static_cast<float>(
          mSolarSystem->pSunLuminousPower.get() /
          (sceneScale * sceneScale * mRadii[0] * mRadii[0] * 4.0 * glm::pi<double>()));
    }

    ambientBrightness = 1.0F;

  } else if (mSun) {
    // For all other bodies we can use the utility methods from the SolarSystem.
    if (mSettings->mGraphics.pEnableHDR.get()) {
      sunIlluminance = static_cast<float>(mSolarSystem->getSunIlluminance(getWorldTransform()[3]));
    }

    sunDirection = mSolarSystem->getSunDirection(getWorldTransform()[3]);
  }

  mShader.SetUniform(mShader.GetUniformLocation("uSunDirection"), sunDirection[0], sunDirection[1],
      sunDirection[2]);
  mShader.SetUniform(mShader.GetUniformLocation("uSunIlluminance"), sunIlluminance);
  mShader.SetUniform(mShader.GetUniformLocation("uAmbientBrightness"), ambientBrightness);

  // Get modelview and projection matrices.
  std::array<GLfloat, 16> glMatMV{};
  std::array<GLfloat, 16> glMatP{};
  glGetFloatv(GL_MODELVIEW_MATRIX, glMatMV.data());
  glGetFloatv(GL_PROJECTION_MATRIX, glMatP.data());
  auto matMV = glm::make_mat4x4(glMatMV.data()) * glm::mat4(getWorldTransform());
  glUniformMatrix4fv(
      mShader.GetUniformLocation("uMatModelView"), 1, GL_FALSE, glm::value_ptr(matMV));
  glUniformMatrix4fv(mShader.GetUniformLocation("uMatProjection"), 1, GL_FALSE, glMatP.data());

  mShader.SetUniform(mShader.GetUniformLocation("uSurfaceTexture"), 0);
  mShader.SetUniform(mShader.GetUniformLocation("uRadii"), static_cast<float>(mRadii[0]),
      static_cast<float>(mRadii[0]), static_cast<float>(mRadii[0]));
  mShader.SetUniform(
      mShader.GetUniformLocation("uFarClip"), cs::utils::getCurrentFarClipDistance());

  mTexture->Bind(GL_TEXTURE0);

  // Draw.
  mSphereVAO.Bind();
  glDrawElements(GL_TRIANGLE_STRIP, (GRID_RESOLUTION_X - 1) * (2 + 2 * GRID_RESOLUTION_Y),
      GL_UNSIGNED_INT, nullptr);
  mSphereVAO.Release();

  // Clean up.
  mTexture->Unbind(GL_TEXTURE0);
  mShader.Release();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleBody::GetBoundingBox(VistaBoundingBox& /*bb*/) {
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::simplebodies
