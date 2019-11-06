////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TextureTracerCPU.hpp"

#include "../../cs-utils/ThreadPool.hpp"
#include "EclipseConstants.hpp"
#include "Photon.hpp"

#include <GL/glew.h>
#include <array>
#include <atomic>
#include <cmath>
#include <glm/glm.hpp>

namespace cs::graphics {

void operator+=(std::atomic<double>& atomic, double value) {
  double old;
  do {
    old = atomic;
  } while (!atomic.compare_exchange_strong(old, old + value));
}

std::vector<FloatPixel> TextureTracerCPU::traceThroughTexture(
    uint32_t ssboPhotons, size_t numPhotons, BodyWithAtmosphere const& body) {
  const double atmosphericRadius = body.meanRadius + body.atmosphere.height;
  const double rectangleHeight =
      atmosphericRadius * TEX_HEIGHT_TO_RADIUS_FACTOR / static_cast<double>(TEX_HEIGHT);

  const double shadowHeight = atmosphericRadius * TEX_HEIGHT_TO_RADIUS_FACTOR;

  const double shadowLength = TEX_SHADOW_LENGTH_FACTOR *
                              (body.orbit.semiMajorAxisSun * atmosphericRadius) /
                              (SUN_RADIUS - atmosphericRadius);

  const double xAxisScalingFactor =
      std::log(shadowLength) / std::log(static_cast<double>(TEX_WIDTH));

  auto horizontalRectangles = std::array<glm::dvec2, TEX_WIDTH>{};

  double xx0 = 0.0;
  for (int x = 0; x < TEX_WIDTH; ++x) {
    const double xx1        = std::pow(static_cast<double>(x + 1), xAxisScalingFactor);
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

  cs::utils::ThreadPool          tp(std::thread::hardware_concurrency());
  std::vector<std::future<void>> tasks(pixels.size());

  for (size_t i = 0; i < pixels.size(); ++i) {
    tasks[i] = tp.enqueue([&, i] {
      for (auto&& a : pixels[i].intensityAtWavelengths)
        a = 0.0;
    });
  }

  for (auto&& task : tasks) {
    task.get();
  }

  struct Rectangle {
    double x;
    double y;
    double w;
    double h;
  };

  auto getRectangleAt = [&](glm::ivec2 const& indices) -> Rectangle {
    glm::dvec2 horRect = horizontalRectangles[indices.x];
    return Rectangle{horRect.x, rectangleHeight * indices.y, horRect.y, rectangleHeight};
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

  auto getRayIntersectAtX = [](PhotonF const& ray, double x) -> double {
    double slope = ray.direction.y / ray.direction.x;
    return slope * (x - ray.position.x) + ray.position.y;
  };

  auto getRayRectangleExitEdge = [&](PhotonF const& ray, Rectangle const& rect) -> glm::ivec2 {
    double intersectHeight = getRayIntersectAtX(ray, rect.x + rect.w);
    if (intersectHeight < rect.y) {
      return glm::ivec2(0, -1);
    } else if (intersectHeight > rect.y + rect.h) {
      return glm::ivec2(0, 1);
    } else {
      return glm::ivec2(1, 0);
    }
  };

  struct Line {
    glm::dvec2 start;
    glm::dvec2 end;
  };

  auto rayLineIntersection = [](PhotonF const& ray, Line const& line) -> std::optional<glm::dvec2> {
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

  auto rayRectangleIntersection = [&](PhotonF const&   ray,
                                      Rectangle const& rect) -> std::pair<glm::dvec2, glm::dvec2> {
    glm::dvec2 topLeft(rect.x, rect.y), topRight(rect.x + rect.w, rect.y);
    glm::dvec2 botLeft(rect.x, rect.y + rect.h), botRight(rect.x + rect.w, rect.y + rect.h);

    std::array<Line, 4> rectEdges{Line{botLeft, topLeft}, Line{topLeft, topRight},
        Line{botRight, botLeft}, Line{topRight, botRight}};

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

  auto rayDistanceInRectangle = [&](PhotonF const& ray, Rectangle const& rect) -> double {
    auto [entry, exit] = rayRectangleIntersection(ray, rect);
    return glm::length(exit - entry);
  };

  std::vector<PhotonF> photons(numPhotons);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPhotons);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(PhotonF) * photons.size(), photons.data());

  const double pixelInAtmosphere = body.atmosphere.height / rectangleHeight;
  const double photonsPerPixel   = photons.size() / pixelInAtmosphere;

  std::vector<std::future<void>> photonTasks(photons.size());

  for (size_t gid = 0; gid < photons.size(); ++gid) {
    photonTasks[gid] = tp.enqueue([&, gid] {
      PhotonF localPhoton = photons[gid];

      if (localPhoton.intensity > 0.0) {
        glm::ivec2 photonTexIndices = getRectangleIdxAt(localPhoton.position);

        while (photonTexIndices.x < TEX_WIDTH && photonTexIndices.y < TEX_HEIGHT &&
               photonTexIndices.x >= 0 && photonTexIndices.y >= 0) {
          Rectangle rect     = getRectangleAt(photonTexIndices);
          double    distance = rayDistanceInRectangle(localPhoton, rect);

          double contrib = (distance / rect.w);

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
  }

  for (auto&& photonTask : photonTasks) {
    photonTask.get();
  }

  std::vector<FloatPixel> resultBuffer(TEX_WIDTH * TEX_HEIGHT);

  std::vector<std::future<void>> bufferTasks(resultBuffer.size());
  for (size_t i = 0; i < resultBuffer.size(); ++i) {
    bufferTasks[i] = tp.enqueue([&, i] {
      FloatPixel   fp{};
      AtomicPixel& ap = pixels[i];

      for (size_t p = 0; p < fp.intensitiesAtWavelengths.size(); ++p) {
        fp.intensitiesAtWavelengths[p] = ap.intensityAtWavelengths[p] / photonsPerPixel;
      }

      resultBuffer[i] = fp;
    });
  }

  for (auto&& task : bufferTasks) {
    task.get();
  }

  return resultBuffer;
}
} // namespace cs::graphics
