////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_TEXTURE_TRACER_CPU_HPP
#define CS_GRAPHICS_TEXTURE_TRACER_CPU_HPP

#include "../../cs-utils/geometry/Line.hpp"
#include "../../cs-utils/geometry/Rectangle.hpp"
#include "BodyProperties.hpp"
#include "Photon.hpp"
#include "TextureTracer.hpp"

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

namespace cs::graphics {
class TextureTracerCPU : public TextureTracer {
 public:
  std::vector<DoublePixel> traceThroughTexture(
      std::variant<GPUBuffer, CPUBuffer> const& photonBuffer, BodyWithAtmosphere const& body);
};

namespace detail {

class GridTracer {
 public:
  GridTracer(double rectangleHeight, double shadowHeight, double shadowLength,
      std::array<glm::dvec2, TEX_WIDTH> const& horizontalRectangles)
      : rectangleHeight(rectangleHeight)
      , shadowHeight(shadowHeight)
      , shadowLength(shadowLength)
      , horizontalRectangles(horizontalRectangles) {
  }

  utils::geom::DRectangle getRectangleAt(glm::ivec2 const& indices);

  int binarySearchForHorizontalRectangle(double x);

  glm::ivec2 getRectangleIdxAt(glm::dvec2 const& location);

  double getRayIntersectAtX(Photon const& ray, double x);

  glm::ivec2 getRayRectangleExitEdge(Photon const& ray, utils::geom::DRectangle const& rect);

 private:
  double const                             rectangleHeight;
  double const                             shadowHeight;
  double const                             shadowLength;
  std::array<glm::dvec2, TEX_WIDTH> const& horizontalRectangles;
};

} // namespace detail

} // namespace cs::graphics

#endif // CS_GRAPHICS_TEXTURE_TRACER_CPU_HPP
