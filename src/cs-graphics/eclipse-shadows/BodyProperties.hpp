////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_BODY_PROPERTIES_HPP
#define CS_GRAPHICS_BODY_PROPERTIES_HPP

#include <ostream>
#include <utility>
#include <vector>

namespace cs::graphics {

/// DocTODO
struct AtmosphereLayer {
  float altitude;             ///< m
  float baseTemperature;      ///< K
  float temperatureLapseRate; ///< K/m
  float baseDensity;          ///< kg/m^3

  friend std::ostream& operator<<(std::ostream& os, const AtmosphereLayer& layer) {
    os << "{altitude: " << layer.altitude << " baseTemperature: " << layer.baseTemperature
       << " temperatureLapseRate: " << layer.temperatureLapseRate
       << " baseDensity: " << layer.baseDensity << "}";
    return os;
  }
};

/// DocTODO
struct SellmeierCoefficients {
  float                                a;
  std::vector<std::pair<float, float>> terms;

  friend std::ostream& operator<<(std::ostream& os, const SellmeierCoefficients& coefficients) {
    os << "{a: " << coefficients.a << " terms: [";
    for (const auto& term : coefficients.terms) {
      os << "{" << term.first << ", " << term.second << "}, ";
    }
    os << "]}";
    return os;
  }
};

/// DocTODO
struct Atmosphere {
  float                        seaLevelMolecularNumberDensity; ///< cm^-3
  float                        molarMass;                      ///< kg/mol
  float                        height;                         ///< m
  std::vector<AtmosphereLayer> layers;
  SellmeierCoefficients        sellmeierCoefficients;

  friend std::ostream& operator<<(std::ostream& os, const Atmosphere& atmosphere) {
    os << "{seaLevelMolecularNumberDensity: " << atmosphere.seaLevelMolecularNumberDensity
       << " molarMass: " << atmosphere.molarMass << " height: " << atmosphere.height
       << " layers: [";

    for (const auto& layer : atmosphere.layers) {
      os << layer << ", ";
    }

    os << "] sellmeierCoefficients: " << atmosphere.sellmeierCoefficients << "}";
    return os;
  }
};

/// DocTODO
struct Orbit {
  // double perihelion;    ///< m
  // double aphelion;      ///< m
  float semiMajorAxisSun; ///< m
  // double eccentricity;

  friend std::ostream& operator<<(std::ostream& os, const Orbit& orbit) {
    os << "semiMajorAxisSun: " << orbit.semiMajorAxisSun;
    return os;
  }
};

struct Body {
  float meanRadius;
  Orbit orbit;
};

struct BodyWithAtmosphere {
  float      gravity;
  float      meanRadius;
  Orbit      orbit;
  Atmosphere atmosphere;

  friend std::ostream& operator<<(std::ostream& os, const BodyWithAtmosphere& atmosphere) {
    os << "gravity: " << atmosphere.gravity << " meanRadius: " << atmosphere.meanRadius
       << " orbit: " << atmosphere.orbit << " atmosphere: " << atmosphere.atmosphere;
    return os;
  }
};

} // namespace cs::graphics

#endif // CS_GRAPHICS_BODY_PROPERTIES_HPP
