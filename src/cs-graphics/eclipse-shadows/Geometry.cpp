////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Geometry.hpp"

#include <cmath>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>

namespace cs::graphics {

double angularRadOfSphere(double distance, double radius) {
  return std::asin(radius / distance);
}

double areaOfCircle(double radius) {
  return glm::pi<double>() * radius * radius;
}

double areaOfCircleIntersection(double radiusSun, double radiusPlanet, double centerDistance) {
  // No intersection
  if (centerDistance >= radiusSun + radiusPlanet)
    return 0.0;

  // One circle fully in the other (total eclipse)
  if (std::min(radiusSun, radiusPlanet) <= std::max(radiusSun, radiusPlanet) - centerDistance)
    return areaOfCircle(radiusPlanet);

  const double d = centerDistance;

  double rrS = radiusSun * radiusSun;
  double rrP = radiusPlanet * radiusPlanet;
  double dd  = d * d;

  double d1 = std::fma(radiusSun, radiusSun, std::fma(-radiusPlanet, radiusPlanet, dd)) / (2 * d);
  double d2 = d - d1;

  // This should be faster o.0
  double fourth = -d2 * std::sqrt(std::fma(-d2, d2, rrP));
  double third  = std::fma(rrP, std::acos(d2 / radiusPlanet), fourth);
  double second = std::fma(-d1, std::sqrt(std::fma(-d1, d1, rrS)), third);
  return std::fma(rrS, std::acos(d1 / radiusSun), second);
}

double enclosingAngle(glm::dvec2 v1, glm::dvec2 v2) {
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

double raySphereDistance(
    glm::dvec2 origin, glm::dvec2 direction, glm::dvec2 center, double radius) {
  glm::dvec2 m = origin - center;
  double     b = dot(m, direction);
  double     c = dot(m, m) - (radius * radius);
  if (c > 0.0 && b > 0.0)
    return -1.0;

  double discriminant = b * b - c;

  // A negative discriminant corresponds to ray missing sphere
  if (discriminant < 0.0)
    return -1.0;

  // Ray now found to intersect sphere, compute smallest t value of intersection
  return std::max(0.0, -b - std::sqrt(discriminant));
}

bool raySphereIntersect(glm::dvec2 origin, glm::dvec2 direction, glm::dvec2 center, double radius) {
  glm::dvec2 m = origin - center;
  double     b = glm::dot(m, direction);
  double     c = glm::dot(m, m) - (radius * radius);
  if (c > 0.0 && b > 0.0)
    return false;

  double discriminant = b * b - c;

  // A negative discriminant corresponds to ray missing sphere
  return discriminant >= 0.0;
}

} // namespace cs::graphics