////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AtmosphereEclipseTextureGenerator.hpp"
#include "../../cs-utils/ThreadPool.hpp"
#include "BlackBodySpectrum.hpp"
#include "EclipseConstants.hpp"
#include "Geometry.hpp"
#include "SimpleEclipseShadowCaster.hpp"
#include "TextureTracerCPU.hpp"
#include <GL/glew.h>
#include <fstream>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>

namespace {

template <size_t SIZE>
std::array<std::array<float, SIZE>, SIZE> generateGaussianKernel() {
  // intialising standard deviation to 1.0
  double sigma = (SIZE / 2) / 2.0;
  double s     = 2.0 * sigma * sigma;

  // sum is for normalization
  double sum = 0.0;

  std::array<std::array<float, SIZE>, SIZE> kernel{};
  int                                       halfSize = SIZE / 2;

  // generating 5x5 kernel
  for (int x = -halfSize; x <= halfSize; x++) {
    for (int y = -halfSize; y <= halfSize; y++) {
      double r                           = glm::sqrt(x * x + y * y);
      kernel[x + halfSize][y + halfSize] = (glm::exp(-(r * r) / s)) / (glm::pi<double>() * s);
      sum += kernel[x + halfSize][y + halfSize];
    }
  }

  // normalising the Kernel
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j)
      kernel[i][j] /= sum;

  return kernel;
}

template <size_t RADIUS>
std::vector<glm::vec4> guassianBlur(
    const std::vector<glm::vec4>& image, int64_t width, int64_t height) {
  const int64_t filterSize   = RADIUS * 2 + 1;
  const int64_t filterRadius = RADIUS;

  const auto filter = generateGaussianKernel<filterSize>();

  auto filterWeight = [&filter](size_t x, size_t y) -> float {
    return filter[x + filterRadius][y + filterRadius];
  };

  std::vector<glm::vec4> output(image.size());

  cs::utils::ThreadPool          tp(std::thread::hardware_concurrency());
  std::vector<std::future<void>> tasks(height);

  for (int64_t y = 0; y < height; ++y) {
    tasks[y] = tp.enqueue([&, y] {
      for (int64_t x = 0; x < width; ++x) {
        glm::vec4 sum(0.0);

        for (int64_t i = -filterRadius; i <= filterRadius; ++i) {
          for (int64_t j = -filterRadius; j <= filterRadius; ++j) {
            float weight = filterWeight(i, j);

            int64_t dx = glm::abs(x + i);
            if (dx >= width) {
              dx = width - 1;
            }

            int64_t dy = glm::abs(y + j);
            if (dy >= height) {
              dy = height - 1;
            }

            glm::vec4 value = image[dy * width + dx];
            sum.r += weight * value.r;
            sum.g += weight * value.g;
            sum.b += weight * value.b;
          }
        }

        output[y * width + x] = sum;
      }
    });
  }

  for (auto&& task : tasks) {
    task.get();
  }

  return output;
}
} // namespace

namespace cs::graphics {
AtmosphereEclipseTextureGenerator::AtmosphereEclipseTextureGenerator()
    : mRNG(/*133713371337 */ std::random_device()())
    , mDistributionWavelength(
          std::uniform_int_distribution<uint32_t>(MIN_WAVELENGTH, MAX_WAVELENGTH))
    , mDistributionBoolean(std::bernoulli_distribution(0.5))
    , mPhotonAtmosphereTracer()
    , mTextureTracer(std::make_unique<TextureTracerCPU>())
    , mColorConverter() {

  mPhotonAtmosphereTracer.init();
  mTextureTracer->init();
  mColorConverter.init();
}

std::string toPPMString(const std::vector<glm::vec4>& data, size_t width, size_t height) {
  std::string output = "P3\n";
  output.append(std::to_string(width) + " " + std::to_string(height) + "\n");
  output.append("65535\n");

  size_t counter = 0;
  for (auto&& pixel : data) {
    output.append(
        std::to_string(lroundf(std::clamp(pixel.x * 25.0f, 0.0f, 1.0f) * 65535.0f)) + " ");
    output.append(
        std::to_string(lroundf(std::clamp(pixel.y * 25.0f, 0.0f, 1.0f) * 65535.0f)) + " ");
    output.append(std::to_string(lroundf(std::clamp(pixel.z * 25.0f, 0.0f, 1.0f) * 65535.0f)));

    if (counter++ == 5) {
      output.append("\n");
      counter = 0;
    } else
      output.append(" ");
  }

  return output;
}

void toPPMFile(const std::vector<glm::vec4>& data, size_t width, size_t height,
    const std::string_view fileName) {
  std::string   output = toPPMString(data, width, height);
  std::ofstream ofStream(fileName.data(), std::ios::out);
  ofStream << output;
  ofStream.close();
}

utils::Texture4f AtmosphereEclipseTextureGenerator::createShadowMap(
    BodyWithAtmosphere const& body, size_t photonCount) {
  std::vector<PhotonF> photons = generatePhotons(photonCount, body);

  uint32_t ssboPhotons;
  glGenBuffers(1, &ssboPhotons);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPhotons);
  glBufferData(
      GL_SHADER_STORAGE_BUFFER, sizeof(PhotonF) * photons.size(), photons.data(), GL_DYNAMIC_COPY);

  mPhotonAtmosphereTracer.traceThroughAtmosphere(ssboPhotons, photons.size(), body);
  auto result = mTextureTracer->traceThroughTexture(ssboPhotons, photons.size(), body);
  std::vector<glm::vec4> texture = mColorConverter.convert(result);

  toPPMFile(texture, TEX_WIDTH, TEX_HEIGHT,
      "eclipse_shadow_" + std::to_string(body.gravity) + ".raw.ppm");

  std::vector<glm::vec4> outputTexture =
      guassianBlur<static_cast<size_t>(TEX_WIDTH * 0.01)>(texture, TEX_WIDTH, TEX_HEIGHT);

  auto shadowTexture = generateShadowTexture({body.meanRadius, body.orbit});
  auto data          = shadowTexture.dataPtr();

  utils::Texture4f resultTexture(TEX_WIDTH, TEX_HEIGHT);

  for (size_t i = 0; i < outputTexture.size(); ++i) {
    resultTexture.dataPtr()[i] = glm::vec4(outputTexture[i].rgb() + data[i].rgb(), 1.0f);
  }

  glDeleteBuffers(1, &ssboPhotons);

  return resultTexture;
}

std::uniform_real_distribution<> angleGenerator(-glm::half_pi<double>(), glm::half_pi<double>());

glm::dvec2 AtmosphereEclipseTextureGenerator::randomPointOnSunSurface(double sunPositionX) {
  const double angle = angleGenerator(mRNG);
  const double x     = std::cos(angle) * SUN_RADIUS;
  const double y     = std::sin(angle) * SUN_RADIUS;

  return glm::dvec2(x + sunPositionX, y);
}

PhotonF AtmosphereEclipseTextureGenerator::emitPhoton(BodyWithAtmosphere const& body) {
  std::uniform_real_distribution<> distributionEarth(0.0, body.atmosphere.height);
  glm::dvec2                       target = {0.0, body.meanRadius + distributionEarth(mRNG)};

  glm::dvec2 startPosition;
  glm::dvec2 direction;
  do {
    startPosition = randomPointOnSunSurface(-body.orbit.semiMajorAxisSun);
    direction     = glm::normalize(target - startPosition);
  } while (raySphereIntersect(
      startPosition, direction, glm::dvec2(-body.orbit.semiMajorAxisSun, 0.0), SUN_RADIUS));

  startPosition += direction * raySphereDistance(startPosition, direction, {0.0, 0.0},
                                   body.meanRadius + body.atmosphere.height);
  uint32_t wavelength = mDistributionWavelength(mRNG);
  float    intensity  = INTENSITY_LUT[wavelength - MIN_WAVELENGTH];
  return {glm::vec2(startPosition), glm::vec2(direction), wavelength, intensity};
}

std::vector<PhotonF> AtmosphereEclipseTextureGenerator::generatePhotons(
    uint32_t count, BodyWithAtmosphere const& body) {
  std::vector<PhotonF> photons(count);

  cs::utils::ThreadPool          tp(std::thread::hardware_concurrency());
  std::vector<std::future<void>> tasks(count);

  for (size_t i = 0; i < count; ++i) {
    tasks[i] = tp.enqueue([&, i] { photons[i] = emitPhoton(body); });
  }

  for (auto&& task : tasks) {
    task.get();
  }

  return photons;
}

} // namespace cs::graphics