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

template <typename T>
struct Photon2 {
  glm::tvec2<T> position;   ///< m
  glm::tvec2<T> direction;  ///< normalized
  uint32_t      wavelength; ///< nm
  T             intensity;  ///< All photons have an average of 1 according to black body spectrum.
};

typedef Photon2<float>  Photon2F;
typedef Photon2<double> Photon2D;

template <typename T>
struct Photon {
  glm::tvec3<T> position;   ///< m              3 * 8 = 24 -> 24
  glm::tvec3<T> direction;  ///< normalized     3 * 8 = 24 -> 48
  T             intensity;  //                  1 * 8 =  8 -> 56
  uint32_t      wavelength; ///< nm             1 * 4 =  4 -> 60
  uint32_t      _padding;   //                  1 * 4 =  4 -> 64
};

typedef Photon<float>  PhotonF;
typedef Photon<double> PhotonD;

} // namespace cs::graphics

#endif // CS_GRAPHICS_PHOTON_HPP
