////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PHOTON_HPP
#define CS_GRAPHICS_PHOTON_HPP

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace cs::graphics {

struct Photon {
  glm::dvec3 position;   ///< m              3 * 8 = 24 -> 24
  glm::dvec3 direction;  ///< normalized     3 * 8 = 24 -> 48
  double     intensity;  //                  1 * 8 =  8 -> 56
  uint32_t   wavelength; ///< nm             1 * 4 =  4 -> 60
  uint32_t   _padding;   //                  1 * 4 =  4 -> 64
};

} // namespace cs::graphics

#endif // CS_GRAPHICS_PHOTON_HPP
