////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_SPHERE_HPP
#define CS_UTIL_GEOMETRY_SPHERE_HPP

#include <glm/glm.hpp>

namespace cs::utils::geom {

template <typename T>
struct Sphere {
  Sphere(glm::tvec3<T> const& center, T radius)
      : center(center)
      , radius(radius) {
  }

  glm::tvec3<T> center;
  T             radius;
};

using FSphere = Sphere<float>;
using DSphere = Sphere<double>;

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_SPHERE_HPP
