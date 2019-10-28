////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PHOTON_HPP
#define CS_GRAPHICS_PHOTON_HPP

#include <glm/vec2.hpp>

namespace cs::graphics {

template <typename T>
struct Photon {
  glm::tvec2<T> position;   ///< m
  glm::tvec2<T> direction;  ///< normalized
  uint32_t      wavelength; ///< nm
  T             intensity;  ///< All photons have an average of 1 according to black body spectrum.
};

typedef Photon<float>  PhotonF;
typedef Photon<double> PhotonD;

} // namespace cs::graphics

#endif // CS_GRAPHICS_PHOTON_HPP
