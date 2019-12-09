////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_ALGORITHMS_HPP
#define CS_UTIL_GEOMETRY_ALGORITHMS_HPP

#include <glm/glm.hpp>

#include "Ray.hpp"
#include "Sphere.hpp"

namespace cs::utils::geom {

template <typename T>
bool rayHitSphere(Ray<3, T> const& ray, Sphere<T> const& sphere) {
  glm::tvec3<T> oc           = ray.origin - sphere.center;
  T             a            = glm::dot(ray.direction, ray.direction);
  T             b            = static_cast<T>(2.0L) * glm::dot(oc, ray.direction);
  T             c            = glm::dot(oc, oc) - sphere.radius * sphere.radius;
  T             discriminant = b * b - static_cast<T>(4.0L) * a * c;
  return (discriminant > static_cast<T>(0.0L));
}

template <typename T>
T raySphereDistance(Ray<3, T> const& ray, Sphere<T> const& sphere) {
  glm::tvec3<T> oc           = ray.origin - sphere.center;
  T             a            = glm::dot(ray.direction, ray.direction);
  T             b            = static_cast<T>(2.0L) * glm::dot(oc, ray.direction);
  T             c            = glm::dot(oc, oc) - sphere.radius * sphere.radius;
  T             discriminant = b * b - static_cast<T>(4.0L) * a * c;
  if (discriminant < static_cast<T>(0.0L)) {
    return static_cast<T>(-1.0L);
  } else {
    return (-b - std::sqrt(discriminant)) / (static_cast<T>(2.0L) * a);
  }
}

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_ALGORITHMS_HPP
