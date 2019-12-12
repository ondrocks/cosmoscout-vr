////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP
#define CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP

#include "AtmosphereTracer.hpp"
#include "BodyProperties.hpp"
#include "LUTPrecalculator.hpp"
#include <boost/mpl/size_t.hpp>
#include <cstdint>
#include <memory>

namespace cs::graphics {
class AtmosphereTracerCPU : public AtmosphereTracer {
 public:
  AtmosphereTracerCPU() = default;

  void init() override;

  std::variant<GPUBuffer, CPUBuffer> traceThroughAtmosphere(
      CPUBuffer& photonBuffer, const BodyWithAtmosphere& body, double xPosition);

  ~AtmosphereTracerCPU() override = default;

 private:
  std::unique_ptr<LUTPrecalculator> mLutPrecalculator;
};
} // namespace cs::graphics
#endif // CS_GRAPHICS_PHOTON_ATMOSPHERE_TRACER_HPP
