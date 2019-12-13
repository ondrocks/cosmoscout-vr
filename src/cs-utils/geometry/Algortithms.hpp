////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_ALGORITHMS_HPP
#define CS_UTIL_GEOMETRY_ALGORITHMS_HPP

#include <glm/ext/scalar_constants.hpp>
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

template <int Size, typename T>
double enclosingAngle(glm::vec<Size, T> v1, glm::vec<Size, T> v2) {
  double a = glm::length(v1);
  double b = glm::length(v2);
  double c = glm::length(v1 - v2);

  double mu;
  if (b >= c) {
    mu = c - (a - b);
  } else {
    mu = b - (a - c);
  }

  double top = ((a - b) + c) * mu;
  double bot = (a + (b + c)) * ((a - c) + b);

  return 2.0 * glm::atan(std::sqrt(top), std::sqrt(bot));
}

template <typename T>
T angularRadOfSphere(T distance, T radius) {
  return std::asin(radius / distance);
}

template <typename T>
T areaOfCircle(T radius) {
  return glm::pi<T>() * radius * radius;
}

template <typename T>
T areaOfCircleIntersection(T radiusSun, T radiusPlanet, T centerDistance) {
  // No intersection
  if (centerDistance >= radiusSun + radiusPlanet)
    return 0.0;

  // One circle fully in the other (total eclipse)
  if (std::min(radiusSun, radiusPlanet) <= std::max(radiusSun, radiusPlanet) - centerDistance)
    return areaOfCircle(radiusPlanet);

  const T d = centerDistance;

  T rrS = radiusSun * radiusSun;
  T rrP = radiusPlanet * radiusPlanet;
  T dd  = d * d;

  T d1 = std::fma(radiusSun, radiusSun, std::fma(-radiusPlanet, radiusPlanet, dd)) / (2.0 * d);
  T d2 = d - d1;

  // This should be faster o.0
  T fourth = -d2 * std::sqrt(std::fma(-d2, d2, rrP));
  T third  = std::fma(rrP, std::acos(d2 / radiusPlanet), fourth);
  T second = std::fma(-d1, std::sqrt(std::fma(-d1, d1, rrS)), third);
  return std::fma(rrS, std::acos(d1 / radiusSun), second);
}

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_ALGORITHMS_HPP
