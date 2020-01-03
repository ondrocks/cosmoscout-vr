////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTILS_GEOMETRY_QUADRILATERAL_HPP
#define CS_UTILS_GEOMETRY_QUADRILATERAL_HPP

#include <array>
#include <glm/glm.hpp>

namespace cs::utils::geom {

template <typename T>
struct Quadrilateral {
  Quadrilateral(glm::tvec2<T> const& a, glm::tvec2<T> const& b, glm::tvec2<T> const& c,
      glm::tvec2<T> const& d)
      : a(a)
      , b(b)
      , c(c)
      , d(d) {
  }

  Quadrilateral(std::array<glm::tvec2<T>, 4> const& corners)
      : corners(corners) {
  }

  union {
    std::array<glm::tvec2<T>, 4> corners;

    struct {
      glm::tvec2<T> a;
      glm::tvec2<T> b;
      glm::tvec2<T> c;
      glm::tvec2<T> d;
    };
  };
};

using FQuadrilateral = Quadrilateral<float>;
using DQuadrilateral = Quadrilateral<double>;
} // namespace cs::utils::geom

#endif // CS_UTILS_GEOMETRY_QUADRILATERAL_HPP
