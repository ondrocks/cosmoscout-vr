////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP
#define CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP

#include "LUTPrecalculator.hpp"

#include <cstdint>
#include <memory>

namespace cs::graphics {

class PhotonAtmosphereTracer {
 public:
  PhotonAtmosphereTracer() = default;

  void init();

  void traceThroughAtmosphere(uint32_t ssboPhotons, size_t numPhotons,
                              core::Settings::BodyProperties const& bodyProperties);

  virtual ~PhotonAtmosphereTracer();

 private:
  uint32_t mProgram;

  struct {
    uint32_t planetRadius;
    uint32_t planetAtmosphericHeight;
    uint32_t planetSeaLevelMolecularNumberDensity;

    uint32_t pass;
    uint32_t passSize;
  } mUniforms{};

  std::unique_ptr<LUTPrecalculator> mLutPrecalculator;
};

} // namespace cs::graphics

#endif // CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP
