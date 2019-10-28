////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_REFRACTIVE_INDEX_PRECALCULATOR_HPP
#define CS_GRAPHICS_REFRACTIVE_INDEX_PRECALCULATOR_HPP

#include "../../cs-core/Settings.hpp"
#include <cstdint>
#include <utility>

namespace cs::graphics {

class LUTPrecalculator {
 public:
  LUTPrecalculator();

  std::pair<uint32_t, uint32_t> createLUT(core::Settings::BodyProperties const& bodyProperties);

  virtual ~LUTPrecalculator();

 private:
  struct Uniforms {
    struct {
      uint32_t uAtmosphericHeight;
      uint32_t uGravity;
      uint32_t uMolarMass;
      uint32_t uSeaLevelMolecularNumberDensity;
    } planet;

    struct {
      uint32_t uA;
      uint32_t uNumTerms;
      uint32_t uTerms[8];
    } sellmeierCoefficients;

  } mUniforms;

  uint32_t mProgram;
};

}

#endif //CS_GRAPHICS_REFRACTIVE_INDEX_PRECALCULATOR_HPP
