////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_COLOR_CONVERTER_HPP
#define CS_GRAPHICS_COLOR_CONVERTER_HPP

#include <cstdint>
#include <glm/vec4.hpp>
#include <vector>

namespace cs::graphics {

struct FloatPixel;

class ColorConverter {
 public:
  ColorConverter() = default;

  void init();

  std::vector<glm::vec4> convert(std::vector<FloatPixel> const& pixel);

  ~ColorConverter();

 private:
  uint32_t mProgram;
};
} // namespace cs::graphics

#endif // CS_GRAPHICS_COLOR_CONVERTER_HPP
