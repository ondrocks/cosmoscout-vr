////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleEclipseShadowCaster.hpp"
#include <cmath>
#include <fstream>
#include <glm/glm.hpp>
#include <string>

#include "../../cs-utils/SimpleTexture.hpp"
#include "../../cs-utils/ThreadPool.hpp"
#include "EclipseConstants.hpp"
#include "Geometry.hpp"

namespace cs::graphics {

void saveGreyscale(const std::string& name, const utils::Texture4f& texture) {

  std::ofstream output(name + ".pgm", std::ios::out | std::ios::trunc);

  output << "P2 " << texture.mWidth << " " << texture.mHeight << " 255\n";

  int linebreakCounter = 0;
  for (int i = 0; i < texture.mWidth; ++i) {
    for (int j = 0; j < texture.mHeight; ++j) {
      output << static_cast<int>(texture.get(j, i).r * 255.0) << " ";

      // pgm doesn't allow more then 70 characters per line. Every value uses up to 4 characters.
      if (linebreakCounter++ == 16) {
        output << "\n";
        linebreakCounter = 0;
      }
    }
  }
}

std::pair<cs::utils::Texture4f, double> generateShadowTexture(Body const& body) {
  const double shadowLength = TEX_SHADOW_LENGTH_FACTOR *
                              (body.orbit.semiMajorAxisSun * body.meanRadius) /
                              (SUN_RADIUS - body.meanRadius);

  const double xAxisScalingExponent =
      std::log(shadowLength) / std::log(static_cast<double>(TEX_WIDTH));

  const double pixSize = body.meanRadius / (TEX_HEIGHT / TEX_HEIGHT_TO_RADIUS_FACTOR);

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
            angularRadOfSphere(pixelDistanceToPlanet, body.meanRadius);

        const glm::dvec2 pixelPositionRelSun(body.orbit.semiMajorAxisSun + xx, y * pixSize);
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

  saveGreyscale("eclipse_shadow_" + std::to_string(body.meanRadius), texture);

  return {texture, xAxisScalingExponent};
}

SimpleEclipseShadowCaster::SimpleEclipseShadowCaster(Body const& body) {
  mRadius                           = body.meanRadius;
  auto [shadowTex, scalingExponent] = generateShadowTexture(body);
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
  int nActiveTexUnit = 0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &nActiveTexUnit);

  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, mShadowTexture);
  glActiveTexture(nActiveTexUnit);
}

void SimpleEclipseShadowCaster::unbind(GLenum textureUnit) {
  int nActiveTexUnit = 0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &nActiveTexUnit);

  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(nActiveTexUnit);
}

} // namespace cs::graphics