////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AtmosphereEclipseTextureGenerator.hpp"
#include "../../cs-utils/filesystem.hpp"
#include "../../cs-utils/geometry/Algortithms.hpp"
#include "../../cs-utils/parallel.hpp"
#include "../../cs-utils/utils.hpp"
#include "AtmosphereTracerCPU.hpp"
#include "AtmosphereTracerGPU.hpp"
#include "BlackBodySpectrum.hpp"
#include "EclipseConstants.hpp"
#include "Geometry.hpp"
#include "SimpleEclipseShadowCaster.hpp"
#include "TextureTracerCPU.hpp"
#include <GL/glew.h>
#include <boost/pool/detail/mutex.hpp>
#include <glm/detail/type_quat.hpp>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

template <size_t SIZE>
std::array<std::array<double, SIZE>, SIZE> generateGaussianKernel() {
  // intialising standard deviation to 1.0
  double sigma = (SIZE / 2) / 2.0;
  double s     = 2.0 * sigma * sigma;

  // sum is for normalization
  double sum = 0.0;

  std::array<std::array<double, SIZE>, SIZE> kernel{};
  int                                        halfSize = SIZE / 2;

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
std::vector<glm::dvec4> guassianBlur(
    const std::vector<glm::dvec4>& image, int64_t width, int64_t height) {
  const int64_t filterSize   = RADIUS * 2 + 1;
  const int64_t filterRadius = RADIUS;

  const auto filter = generateGaussianKernel<filterSize>();

  auto filterWeight = [&filter, filterRadius](size_t x, size_t y) -> double {
    return filter[x + filterRadius][y + filterRadius];
  };

  std::vector<glm::dvec4> output(image.size());

  cs::utils::executeParallel(height, [&](size_t y) {
    for (int64_t x = 0; x < width; ++x) {
      glm::dvec4 sum(0.0);

      for (int64_t i = -filterRadius; i <= filterRadius; ++i) {
        for (int64_t j = -filterRadius; j <= filterRadius; ++j) {
          double weight = filterWeight(i, j);

          int64_t dx = glm::abs(x + i);
          if (dx >= width) {
            dx = width - 1;
          }

          int64_t dy = glm::abs(y + j);
          if (dy >= height) {
            dy = height - 1;
          }

          glm::dvec4 value = image[dy * width + dx];
          sum.r += weight * value.r;
          sum.g += weight * value.g;
          sum.b += weight * value.b;
        }
      }

      output[y * width + x] = sum;
    }
  });

  return output;
}
} // namespace

namespace cs::graphics {
AtmosphereEclipseTextureGenerator::AtmosphereEclipseTextureGenerator()
    : mRNG(/*133713371337 */ std::random_device()())
    , mDistributionWavelength(
          std::uniform_int_distribution<uint32_t>(MIN_WAVELENGTH, MAX_WAVELENGTH))
    , mDistributionBoolean(std::bernoulli_distribution(0.5))
    , mAtmosphereTracer(std::make_unique<AtmosphereTracerCPU>())
    , mTextureTracer(std::make_unique<TextureTracerCPU>())
    , mColorConverter() {

  mAtmosphereTracer->init();
  mTextureTracer->init();
  mColorConverter.init();
}

std::string toPPMString(const std::vector<glm::dvec4>& data, size_t width, size_t height) {
  std::string output = "P3\n";
  output.append(std::to_string(width) + " " + std::to_string(height) + "\n");
  output.append("65535\n");

  size_t counter = 0;
  for (auto&& pixel : data) {
    output.append(std::to_string(lround(std::clamp(pixel.x * 25.0, 0.0, 1.0) * 65535.0)) + " ");
    output.append(std::to_string(lround(std::clamp(pixel.y * 25.0, 0.0, 1.0) * 65535.0)) + " ");
    output.append(std::to_string(lround(std::clamp(pixel.z * 25.0, 0.0, 1.0) * 65535.0)));

    if (counter++ == 5) {
      output.append("\n");
      counter = 0;
    } else
      output.append(" ");
  }

  return output;
}

void toPPMFile(const std::vector<glm::dvec4>& data, size_t width, size_t height,
    const std::string_view fileName) {
  std::string   output = toPPMString(data, width, height);
  std::ofstream ofStream(fileName.data(), std::ios::out);
  ofStream << output;
  ofStream.close();
}

utils::Texture4f AtmosphereEclipseTextureGenerator::createShadowMap(
    BodyWithAtmosphere const& body, size_t photonCount) {
  std::vector<Photon> photons = generatePhotons(photonCount, body);

  std::vector<glm::dvec3> positions(photons.size());
  for (int i = 0; i < photons.size(); ++i) {
    positions[i] = (photons[i].position / 6371000.0) * 20.0;
  }

  auto objString = utils::verticesToObjString(positions);
  utils::filesystem::saveToFile(objString, "photon_positions_enter.obj");

  double rOcc               = body.meanRadius + body.atmosphere.height;
  double xOcc               = (rOcc * (SUN_RADIUS + rOcc)) / body.orbit.semiMajorAxisSun;
  auto [photonBuffer, time] = utils::measureTimeSeconds<std::variant<GPUBuffer, CPUBuffer>>(
      [&] { return mAtmosphereTracer->traceThroughAtmosphere(photons, body, xOcc); });

  std::cout << "time: " << time << " seconds" << std::endl;

  std::exit(0);

  if (std::holds_alternative<CPUBuffer>(photonBuffer)) {
    for (int i = 0; i < std::get<CPUBuffer>(photonBuffer).size(); ++i) {
      positions[i] = (std::get<CPUBuffer>(photonBuffer)[i].position / 6371000.0) * 20.0;
    }
  } else {
    GPUBuffer gpuBuffer = std::get<GPUBuffer>(photonBuffer);
    photons             = CPUBuffer(gpuBuffer.size);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuffer.buffer);
    glGetBufferSubData(
        GL_SHADER_STORAGE_BUFFER, 0, sizeof(Photon) * gpuBuffer.size, photons.data());

    for (int i = 0; i < photons.size(); ++i) {
      positions[i] = (photons[i].position / 6371000.0) * 20.0;
    }
  }

  objString = utils::verticesToObjString(positions);
  utils::filesystem::saveToFile(objString, "photon_positions_exit.obj");

  auto                    result  = mTextureTracer->traceThroughTexture(photonBuffer, body);
  std::vector<glm::dvec4> texture = mColorConverter.convert(result);

  toPPMFile(texture, TEX_WIDTH, TEX_HEIGHT,
      "eclipse_shadow_" + std::to_string(body.gravity) + ".raw.ppm");

  std::vector<glm::dvec4> outputTexture =
      guassianBlur<static_cast<size_t>(TEX_WIDTH * 0.01)>(texture, TEX_WIDTH, TEX_HEIGHT);

  auto shadowTexture = generateShadowTexture({body.meanRadius, body.orbit});
  auto data          = shadowTexture.dataPtr();

  utils::Texture4f resultTexture(TEX_WIDTH, TEX_HEIGHT);

  for (size_t i = 0; i < outputTexture.size(); ++i) {
    resultTexture.dataPtr()[i] =
        glm::vec4(outputTexture[i].rgb() + glm::dvec3(data[i].rgb()), 1.0f);
  }

  std::exit(0);

  return resultTexture;
}

/// Calculates the limb darkening between the radii 0 - 1.
double calculateLimbDarkening(double radius) {
  return (1.0 - 0.6 * (1.0 - std::sqrt(1.0 - radius * radius))) / 0.8;
}

std::vector<Photon> AtmosphereEclipseTextureGenerator::generatePhotons(
    uint32_t count, BodyWithAtmosphere const& body) {
  // 1. calculate target area
  double rOcc  = body.meanRadius + body.atmosphere.height;
  double rOcc2 = rOcc * rOcc;

  double r2 = (rOcc + SUN_RADIUS) * (rOcc + SUN_RADIUS);

  double d  = body.orbit.semiMajorAxisSun;
  double d2 = d * d;

  double a    = (rOcc2 * std::sqrt(d2 - r2) * d) / (-rOcc * r2 + d2 * rOcc) - body.meanRadius;
  double xOcc = (rOcc * (SUN_RADIUS + rOcc)) / d;

  utils::geom::DSphere sphereBody({xOcc, 0.0, 0.0}, body.meanRadius);
  utils::geom::DSphere sphereAtmosphere({xOcc, 0.0, 0.0}, body.meanRadius + body.atmosphere.height);

  double yLower = body.meanRadius;
  double yUpper = body.meanRadius + a;

  std::uniform_real_distribution<double> targetDistribution(yLower, yUpper);

  std::vector<Photon> photons;
  photons.reserve(count);

  std::mutex lock{};

  // 2. for each i in count
  cs::utils::executeParallel(count, [&](size_t i) {
    // 3. sample random point in target area,
    glm::dvec3 target(xOcc, targetDistribution(mRNG), 0.0);

    // 4. sample random point on suns surface
    double     xSun = xOcc - d;
    glm::dvec3 sunCenter(-xSun, 0.0, 0.0);
    double     angularRadSun = std::asin(SUN_RADIUS / glm::length(target - sunCenter));

    std::uniform_real_distribution<double> angleRng(-angularRadSun, angularRadSun);
    glm::dvec3                             randYawPitch{};
    do {
      randYawPitch = glm::dvec3(0.0, angleRng(mRNG), angleRng(mRNG));
    } while (glm::length(randYawPitch) > angularRadSun);

    glm::dvec3 aimingVector = target - sunCenter;
    glm::dquat rotation(randYawPitch);
    aimingVector         = rotation * aimingVector;
    glm::dvec3 direction = -glm::normalize(aimingVector);
    glm::dvec3 origin    = target + aimingVector;

    utils::geom::DRay3 photonRay(origin, direction);

    // 5. validate resulting ray to ensure it can pass through atmosphere
    if (utils::geom::rayHitSphere(photonRay, sphereBody) ||
        !utils::geom::rayHitSphere(photonRay, sphereAtmosphere)) {
      return;
    }

    // 6. calculate limb darkening for start point
    double limbDarkening = calculateLimbDarkening(glm::length(randYawPitch) / angularRadSun);

    // 7. get random wavelength in visible spectrum
    uint32_t wavelength = mDistributionWavelength(mRNG);
    double   intensity  = INTENSITY_LUT[wavelength - MIN_WAVELENGTH];

    // 8. from wavelength and limb darkening get an intensity
    double intensityAdjusted = limbDarkening * intensity;

    // 9. shoot photon on to atmosphere
    double distanceToAtmosphere = utils::geom::raySphereDistance(photonRay,
        utils::geom::DSphere(sphereBody.center, body.meanRadius + body.atmosphere.height));

    glm::dvec3 startPosition = origin + direction * (distanceToAtmosphere + 10.0);

    if (glm::length(startPosition - sphereBody.center) <= sphereAtmosphere.radius) {
      std::lock_guard<std::mutex> guard(lock);
      photons.emplace_back(startPosition, direction, intensityAdjusted, wavelength);
    }
  });

  std::cout << "Number of Photons send: " << photons.size() << std::endl;

  return photons;
}

} // namespace cs::graphics