////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BlackBodySpectrum.hpp"

namespace cs::graphics {
double calculateLimbDarkening(double radius) {
  return (1.0 - 0.6 * (1.0 - std::sqrt(1.0 - radius * radius))) / 0.8;
}
} // namespace cs::graphics