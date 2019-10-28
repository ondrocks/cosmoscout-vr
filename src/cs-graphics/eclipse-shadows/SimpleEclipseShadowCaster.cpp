////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleEclipseShadowCaster.hpp"
#include <cmath>
#include <glm/glm.hpp>

#include "../../cs-utils/SimpleTexture.hpp"
#include "../../cs-utils/ThreadPool.hpp"
#include "EclipseConstants.hpp"
#include "Geometry.hpp"

namespace cs::graphics {

std::pair<cs::utils::Texture4f, double> generateShadowTexture(
    core::Settings::BodyProperties const& bodyProperties) {
  const double shadowLength = TEX_SHADOW_LENGTH_FACTOR *
                              (bodyProperties.semiMajorAxis * bodyProperties.meanRadius) /
                              (SUN_RADIUS - bodyProperties.meanRadius);

  const double xAxisScalingExponent =
      std::log(shadowLength) / std::log(static_cast<double>(TEX_WIDTH));

  const double pixSize = bodyProperties.meanRadius / (TEX_HEIGHT / TEX_HEIGHT_TO_RADIUS_FACTOR);

  cs::utils::Texture4f texture(TEX_WIDTH, TEX_HEIGHT);

  cs::utils::ThreadPool tp(std::thread::hardware_concurrency());

  std::vector<std::future<void>> tasks(TEX_HEIGHT);
  for (size_t y = 0; y < TEX_HEIGHT; ++y) {
    tasks[y] = tp.enqueue([&, y] {
      for (size_t x = 0; x < TEX_WIDTH; ++x) {
        const double     xx = std::pow(static_cast<double>(x), xAxisScalingExponent);
        const glm::dvec2 pixelPositionRelPlanet(xx, y * pixSize);
        const glm::dvec2 pixelPositionRelPlanetNorm = glm::normalize(pixelPositionRelPlanet);
        const double     pixelDistanceToPlanet      = glm::length(pixelPositionRelPlanet);
        const double     planetAngularRadius =
            angularRadOfSphere(pixelDistanceToPlanet, bodyProperties.meanRadius);

        const glm::dvec2 pixelPositionRelSun(bodyProperties.semiMajorAxis + xx, y * pixSize);
        const glm::dvec2 pixelPositionRelSunNorm = glm::normalize(pixelPositionRelSun);
        const double     pixelDistanceToSun      = glm::length(pixelPositionRelSun);
        const double     sunAngularRadius = angularRadOfSphere(pixelDistanceToSun, SUN_RADIUS);
        const double     sunSolidAngle    = areaOfCircle(sunAngularRadius);

        const double angularDistanceToSun =
            enclosingAngle(pixelPositionRelSunNorm, pixelPositionRelPlanetNorm);

        const double sunHiddenPart =
            areaOfCircleIntersection(sunAngularRadius, planetAngularRadius, angularDistanceToSun);

        const double visibleArea         = sunSolidAngle - sunHiddenPart;
        double       visibleAreaRelative = std::clamp(visibleArea / sunSolidAngle, 0.0, 1.0);

        if (std::isnan(visibleAreaRelative))
          visibleAreaRelative = 0.0;

        texture.set(x, y, glm::vec4(static_cast<float>(visibleAreaRelative)));
      }
    });
  }

  for (auto&& task : tasks) {
    task.get();
  }

  return {texture, xAxisScalingExponent};
}

SimpleEclipseShadowCaster::SimpleEclipseShadowCaster(
    core::Settings::BodyProperties const& bodyProperties) {
  mRadius                           = bodyProperties.meanRadius;
  auto [shadowTex, scalingExponent] = generateShadowTexture(bodyProperties);
  mScalingExponent                  = scalingExponent;

  glGenTextures(1, &mShadowTexture);
  glBindTexture(GL_TEXTURE_2D, mShadowTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, shadowTex.mWidth, shadowTex.mHeight, 0, GL_RGBA,
      GL_FLOAT, shadowTex.dataPtr());
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

SimpleEclipseShadowCaster::~SimpleEclipseShadowCaster() {
  glDeleteTextures(1, &mShadowTexture);
}

void SimpleEclipseShadowCaster::bind(GLenum textureUnit) {
  glBindTexture(textureUnit, mShadowTexture);
}

void SimpleEclipseShadowCaster::unbind(GLenum textureUnit) {
  glBindTexture(textureUnit, 0);
}

} // namespace cs::graphics