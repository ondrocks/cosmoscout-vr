////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_GEOMETRY_HPP
#define CS_GRAPHICS_GEOMETRY_HPP

#include <glm/glm.hpp>

namespace cs::graphics {
double angularRadOfSphere(double distance, double radius);
double areaOfCircle(double radius);
double areaOfCircleIntersection(double radiusSun, double radiusPlanet, double centerDistance);
double enclosingAngle(glm::dvec2 v1, glm::dvec2 v2);

double raySphereDistance(glm::dvec2 origin, glm::dvec2 direction, glm::dvec2 center, double radius);
bool raySphereIntersect(glm::dvec2 origin, glm::dvec2 direction, glm::dvec2 center, double radius);
} // namespace cs::graphics

#endif // CS_GRAPHICS_GEOMETRY_HPP
