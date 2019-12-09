////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_LINE_HPP
#define CS_UTIL_GEOMETRY_LINE_HPP

#include <glm/glm.hpp>

namespace cs::utils::geom {

template <uint8_t Dimension, typename T>
struct Line {
  glm::vec<Dimension, T> start;
  glm::vec<Dimension, T> end;
};

using DLine2 = Line<2, double>;
using DLine3 = Line<3, double>;

using FLine2 = Line<2, float>;
using FLine3 = Line<3, float>;

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_LINE_HPP
