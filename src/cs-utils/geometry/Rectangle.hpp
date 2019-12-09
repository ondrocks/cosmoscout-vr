////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_RECTANGLE_HPP
#define CS_UTIL_GEOMETRY_RECTANGLE_HPP

#include <glm/glm.hpp>

namespace cs::utils::geom {

template <typename T>
struct Rectangle {
  Rectangle(T x, T y, T width, T height)
      : x(x)
      , y(y)
      , width(width)
      , height(height) {
  }

  Rectangle(glm::tvec2<T> const& position, glm::tvec2<T> const& dimensions)
      : position(position)
      , dimensions(dimensions) {
  }

  union {
    struct {
      T x;
      T y;
    };

    glm::tvec2<T> position;
  };

  union {
    struct {
      T width;
      T height;
    };

    glm::tvec2<T> dimensions;
  };
};

using FRectangle = Rectangle<float>;
using DRectangle = Rectangle<double>;

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_RECTANGLE_HPP
