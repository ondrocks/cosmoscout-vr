////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_RAY_HPP
#define CS_UTIL_GEOMETRY_RAY_HPP

#include <glm/glm.hpp>

namespace cs::utils::geom {

template <int32_t Dimensions, typename T>
struct Ray {
  Ray(glm::vec<Dimensions, T> const& origin, glm::vec<Dimensions, T> const& direction)
      : origin(origin)
      , direction(glm::normalize(direction)) {
  }

  glm::vec<Dimensions, T> origin;
  glm::vec<Dimensions, T> direction;
};

using FRay2 = Ray<2, float>;
using DRay2 = Ray<2, double>;

using FRay3 = Ray<3, float>;
using DRay3 = Ray<3, double>;

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_RAY_HPP
