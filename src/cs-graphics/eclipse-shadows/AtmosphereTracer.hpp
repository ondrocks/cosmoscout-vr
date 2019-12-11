////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ATMOSPHERE_TRACER_HPP
#define CS_GRAPHICS_ATMOSPHERE_TRACER_HPP

#include "BodyProperties.hpp"
#include "EclipseConstants.hpp"
#include "Photon.hpp"
#include <variant>

namespace cs::graphics {

class AtmosphereTracer {
 public:
  virtual void                               init(){};
  virtual std::variant<GPUBuffer, CPUBuffer> traceThroughAtmosphere(
      CPUBuffer& photonBuffer, BodyWithAtmosphere const& body, double xPosition) = 0;

  virtual ~AtmosphereTracer() = default;
};

} // namespace cs::graphics

#endif // CS_GRAPHICS_ATMOSPHERE_TRACER_HPP
