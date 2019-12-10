////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ECLIPSE_CONSTANTS_HPP
#define CS_GRAPHICS_ECLIPSE_CONSTANTS_HPP

#include <array>
#include <cstdint>

namespace cs::graphics {
double const SUN_RADIUS = 695'510'000.0; ///< m

uint32_t const MIN_WAVELENGTH  = 390u; ///< nm
uint32_t const MAX_WAVELENGTH  = 749u; ///< nm
uint32_t const NUM_WAVELENGTHS = MAX_WAVELENGTH - MIN_WAVELENGTH + 1;

double const TEX_HEIGHT_TO_RADIUS_FACTOR = 4.0;
double const TEX_SHADOW_LENGTH_FACTOR    = 8.0;
double const TEX_SHADOW_WIDTH_EXPONENT   = 2.0;

// TODO make configurable
uint32_t const TEX_WIDTH   = 256u;
uint32_t const TEX_HEIGHT  = TEX_WIDTH;
uint32_t const NUM_PHOTONS = 100'000;

struct DoublePixel {
  std::array<double, NUM_WAVELENGTHS> intensitiesAtWavelengths;
};
} // namespace cs::graphics

#endif // CS_GRAPHICS_ECLIPSE_CONSTANTS_HPP
