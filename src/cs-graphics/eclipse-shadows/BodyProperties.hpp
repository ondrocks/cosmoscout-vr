////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_BODY_PROPERTIES_HPP
#define CS_GRAPHICS_BODY_PROPERTIES_HPP

#include <utility>
#include <vector>

namespace cs::graphics {

/// DocTODO
struct AtmosphereLayer {
  double altitude;             ///< m
  double baseTemperature;      ///< K
  double temperatureLapseRate; ///< K/m
  double baseDensity;          ///< kg/m^3
};

/// DocTODO
struct SellmeierCoefficients {
  double                                 a;
  std::vector<std::pair<double, double>> terms;
};

/// DocTODO
struct Atmosphere {
  double                       seaLevelMolecularNumberDensity; ///< cm^-3
  double                       molarMass;                      ///< kg/mol
  double                       height;                         ///< m
  std::vector<AtmosphereLayer> layers;
  SellmeierCoefficients        sellmeierCoefficients;
};

/// DocTODO
struct Orbit {
  // double perihelion;    ///< m
  // double aphelion;      ///< m
  double semiMajorAxisSun; ///< m
  // double eccentricity;
};

struct Body {
  double meanRadius;
  Orbit orbit;
};

struct BodyWithAtmosphere {
  double gravity;
  double meanRadius;
  Orbit orbit;
  Atmosphere atmosphere;
};
}

#endif // CS_GRAPHICS_BODY_PROPERTIES_HPP
