////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_LINE_SEGMENT_HPP
#define CS_UTIL_GEOMETRY_LINE_SEGMENT_HPP

#include <glm/glm.hpp>

namespace cs::utils::geom {

template <uint8_t Size, typename T>
struct LineSegment {
  LineSegment(glm::vec<Size, T> start, glm::vec<Size, T> end)
      : start(start)
      , end(end) {
  }

  union {
    glm::mat<2, Size, T> mat;

    struct {
      union {
        glm::vec<Size, T> start;
        glm::vec<Size, T> a;
        glm::vec<Size, T> x1;
      };

      union {
        glm::vec<Size, T> end;
        glm::vec<Size, T> b;
        glm::vec<Size, T> x2;
      };
    };
  };
};

template <typename T>
using TLineSegment2 = LineSegment<2, T>;

template <typename T>
using TLineSegment3 = LineSegment<3, T>;

using DLineSegment2 = LineSegment<2, double>;
using DLineSegment3 = LineSegment<3, double>;

using FLineSegment2 = LineSegment<2, float>;
using FLineSegment3 = LineSegment<3, float>;

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_LINE_SEGMENT_HPP
