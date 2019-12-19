////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../src/cs-graphics/eclipse-shadows/Photon.hpp"
#include "../../../src/cs-utils/doctest.hpp"

namespace cs::graphics {

TEST_CASE("cs::graphics::Photon") {
  CHECK_EQ(sizeof(Photon), 64);
}

} // namespace cs::graphics