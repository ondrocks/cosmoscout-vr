////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TextureTracerCPU.hpp"

#include "../../cs-utils/geometry/Line.hpp"
#include "../../cs-utils/geometry/Rectangle.hpp"
#include "../../cs-utils/parallel.hpp"
#include "EclipseConstants.hpp"
#include "Photon.hpp"

#include <GL/glew.h>
#include <array>
#include <atomic>
#include <cmath>
#include <glm/glm.hpp>
#include <optional>

namespace cs::graphics {

void operator+=(std::atomic<double>& atomic, double value) {
  double old;
  do {
    old = atomic;
  } while (!atomic.compare_exchange_strong(old, old + value));
}

std::vector<DoublePixel> TextureTracerCPU::traceThroughTexture(
    std::variant<GPUBuffer, CPUBuffer> const& photonBuffer, BodyWithAtmosphere const& body) {

  const double atmosphericRadius = body.meanRadius + body.atmosphere.height;
  const double rectangleHeight =
      atmosphericRadius * TEX_HEIGHT_TO_RADIUS_FACTOR / static_cast<double>(TEX_HEIGHT);

  const double shadowHeight = atmosphericRadius * TEX_HEIGHT_TO_RADIUS_FACTOR;

  const double shadowLength = TEX_SHADOW_LENGTH_FACTOR *
                              (body.orbit.semiMajorAxisSun * atmosphericRadius) /
                              (SUN_RADIUS - atmosphericRadius);

  auto horizontalRectangles = std::array<glm::dvec2, TEX_WIDTH>{};

  double xx0 = 0.0;
  for (int x = 0; x < TEX_WIDTH; ++x) {
    const double xx1 =
        std::pow(static_cast<double>(x + 1) / TEX_WIDTH, TEX_SHADOW_WIDTH_EXPONENT) * shadowLength;
    horizontalRectangles[x] = glm::dvec2(xx0, xx1 - xx0);
    xx0                     = xx1;
  }

  struct AtomicPixel {
    std::array<std::atomic<double>, NUM_WAVELENGTHS> intensityAtWavelengths{};

    void addAtWavelength(uint32_t wavelength, double value) {
      uint32_t index = wavelength - MIN_WAVELENGTH;
      intensityAtWavelengths[index] += value;
    }
  };

  std::vector<AtomicPixel> pixels(TEX_WIDTH * TEX_HEIGHT);
  cs::utils::executeParallel(pixels.size(), [&](size_t i) {
    for (auto&& a : pixels[i].intensityAtWavelengths)
      a = 0.0;
  });

  auto getRectangleAt = [&](glm::ivec2 const& indices) -> utils::geom::DRectangle {
    glm::dvec2 horRect = horizontalRectangles[indices.x];
    return utils::geom::DRectangle(
        horRect.x, rectangleHeight * indices.y, horRect.y, rectangleHeight);
  };

  auto binarySearchForHorizontalRectangle = [&](double x) -> int {
    int left  = 0;
    int right = static_cast<int>(TEX_WIDTH) - 1;

    while (left <= right) {
      int        middle = (left + right) / 2;
      glm::dvec2 rectM  = horizontalRectangles[middle];
      if (rectM.x + rectM.y < x) {
        left = middle + 1;
      } else if (rectM.x > x) {
        right = middle - 1;
      } else
        return middle;
    }

    // outside of grid (should never happen in any reasonable scenario...)
    return static_cast<int>(TEX_WIDTH);
  };

  auto getRectangleIdxAt = [&](glm::dvec2 const& location) -> glm::ivec2 {
    if (location.x >= 0 && location.x < shadowLength && location.y >= 0 &&
        location.y < shadowHeight) {
      int x = binarySearchForHorizontalRectangle(location.x);
      int y = int(location.y / rectangleHeight);
      return glm::ivec2(x, y);
    } else
      return glm::ivec2(TEX_WIDTH, TEX_HEIGHT);
  };

  // TODO
  auto getRayIntersectAtX = [](Photon const& ray, double x) -> double {
    double slope = ray.direction.y / ray.direction.x;
    return slope * (x - ray.position.x) + ray.position.y;
  };

  auto getRayRectangleExitEdge = [&](Photon const&                  ray,
                                     utils::geom::DRectangle const& rect) -> glm::ivec2 {
    double intersectHeight = getRayIntersectAtX(ray, rect.x + rect.width);
    if (intersectHeight < rect.y) {
      return glm::ivec2(0, -1);
    } else if (intersectHeight > rect.y + rect.height) {
      return glm::ivec2(0, 1);
    } else {
      return glm::ivec2(1, 0);
    }
  };

  auto rayLineIntersection = [](Photon const&               ray,
                                 utils::geom::DLine2 const& line) -> std::optional<glm::dvec2> {
    glm::dvec2 v1 = glm::dvec2(ray.position) - line.start;
    glm::dvec2 v2 = line.end - line.start;
    glm::dvec2 v3 = glm::dvec2(-ray.direction.y, ray.direction.x);

    double dist = 0.0;

    double dot = glm::dot(v2, v3);
    if (glm::abs(dot) < 0.000001)
      return std::nullopt;

    double t1 = (v1.x * v2.y - v1.y * v2.x) / dot;
    double t2 = glm::dot(v1, v3) / dot;

    if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0))
      return std::make_optional<glm::dvec2>(
          glm::dvec2(ray.position) + (glm::dvec2(ray.direction) * t1));
    else
      return std::nullopt;
  };

  auto rayRectangleIntersection =
      [&](Photon const&                  ray,
          utils::geom::DRectangle const& rect) -> std::pair<glm::dvec2, glm::dvec2> {
    glm::dvec2 topLeft(rect.position), topRight(rect.x + rect.width, rect.y);
    glm::dvec2 botLeft(rect.x, rect.y + rect.height),
        botRight(rect.x + rect.width, rect.y + rect.height);

    std::array<utils::geom::DLine2, 4> rectEdges{utils::geom::DLine2{botLeft, topLeft},
        utils::geom::DLine2{topLeft, topRight}, utils::geom::DLine2{botRight, botLeft},
        utils::geom::DLine2{topRight, botRight}};

    glm::dvec2 entry;
    glm::dvec2 exit;

    bool entryFound = false;
    for (const auto& edge : rectEdges) {
      if (auto point = rayLineIntersection(ray, edge)) {
        if (!entryFound) {
          entry      = point.value();
          entryFound = true;
        } else {
          exit = point.value();
          break;
        }
      }
    }

    return std::pair<glm::dvec2, glm::dvec2>(entry, exit);
  };

  auto rayDistanceInRectangle = [&](Photon const&                  ray,
                                    utils::geom::DRectangle const& rect) -> double {
    auto [entry, exit] = rayRectangleIntersection(ray, rect);
    return glm::length(exit - entry);
  };

  CPUBuffer photons;
  if (std::holds_alternative<GPUBuffer>(photonBuffer)) {
    GPUBuffer gpuBuffer = std::get<GPUBuffer>(photonBuffer);
    photons             = CPUBuffer(gpuBuffer.size);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuffer.buffer);
    glGetBufferSubData(
        GL_SHADER_STORAGE_BUFFER, 0, sizeof(Photon) * gpuBuffer.size, photons.data());

    glDeleteBuffers(1, &gpuBuffer.buffer);
  } else {
    photons = std::get<CPUBuffer>(photonBuffer);
  }

  const double pixelInAtmosphere = body.atmosphere.height / rectangleHeight;
  const double photonsPerPixel   = photons.size() / pixelInAtmosphere;

  cs::utils::executeParallel(photons.size(), [&](size_t gid) {
    Photon localPhoton = photons[gid];

    if (localPhoton.intensity > 0.0) {
      glm::ivec2 photonTexIndices = getRectangleIdxAt(localPhoton.position);

      while (photonTexIndices.x < TEX_WIDTH && photonTexIndices.y < TEX_HEIGHT &&
             photonTexIndices.x >= 0 && photonTexIndices.y >= 0) {
        utils::geom::DRectangle rect     = getRectangleAt(photonTexIndices);
        double                  distance = rayDistanceInRectangle(localPhoton, rect);

        double contrib = (distance / rect.width);

        pixels[photonTexIndices.y * TEX_WIDTH + photonTexIndices.x].addAtWavelength(
            localPhoton.wavelength, localPhoton.intensity * contrib);

        glm::ivec2 dir = getRayRectangleExitEdge(localPhoton, rect);
        photonTexIndices += dir;

        // When the ray goes out of bounds on the bottom then mirror it to simulate rays
        // coming from the other side of the planet. This works because of the rotational
        // symmetry of the system.
        if (photonTexIndices.y < 0) {
          photonTexIndices.y = 0;
          localPhoton.position.y *= -1.0;
          localPhoton.direction.y *= -1.0;
        }
      }
    }
  });

  std::vector<DoublePixel> resultBuffer(TEX_WIDTH * TEX_HEIGHT);

  cs::utils::executeParallel(resultBuffer.size(), [&](size_t i) {
    DoublePixel  fp{};
    AtomicPixel& ap = pixels[i];

    for (size_t p = 0; p < fp.intensitiesAtWavelengths.size(); ++p) {
      fp.intensitiesAtWavelengths[p] = ap.intensityAtWavelengths[p] / photonsPerPixel;
    }

    resultBuffer[i] = fp;
  });

  return resultBuffer;
}
} // namespace cs::graphics
