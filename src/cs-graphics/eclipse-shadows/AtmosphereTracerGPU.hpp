////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP
#define CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP

#include "AtmosphereTracer.hpp"
#include "BodyProperties.hpp"
#include "EclipseConstants.hpp"

#include <cstdint>
#include <memory>

namespace cs::graphics {

class LUTPrecalculator;

class AtmosphereTracerGPU : public AtmosphereTracer {
 public:
  AtmosphereTracerGPU() = default;

  void init() override;

  std::variant<GPUBuffer, CPUBuffer> traceThroughAtmosphere(
      CPUBuffer const& photonBuffer, BodyWithAtmosphere const& body, double xPosition);

  ~AtmosphereTracerGPU() override;

 private:
  uint32_t mProgram;

  struct {
    uint32_t planetXPosition;
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
