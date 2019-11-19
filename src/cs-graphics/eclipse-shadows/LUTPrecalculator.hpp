////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_REFRACTIVE_INDEX_PRECALCULATOR_HPP
#define CS_GRAPHICS_REFRACTIVE_INDEX_PRECALCULATOR_HPP

#include "BodyProperties.hpp"
#include <cstdint>
#include <utility>

namespace cs::graphics {

class LUTPrecalculator {
 public:
  LUTPrecalculator();

  std::pair<uint32_t, uint32_t> createLUT(BodyWithAtmosphere const& body);

  virtual ~LUTPrecalculator();

 private:
  struct Uniforms {
    struct {
      int32_t uAtmosphericHeight;
      int32_t uGravity;
      int32_t uMolarMass;
    } planet;

    struct {
      int32_t uA;
      int32_t uNumTerms;
      int32_t uTerms[8];
    } sellmeierCoefficients;

  } mUniforms;

  uint32_t mProgram;
};

} // namespace cs::graphics

#endif // CS_GRAPHICS_REFRACTIVE_INDEX_PRECALCULATOR_HPP
