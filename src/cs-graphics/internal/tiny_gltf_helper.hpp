////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_TINY_GLTF_HELPER
#define CS_GRAPHICS_TINY_GLTF_HELPER
#include <GL/glew.h>
#include <tiny_gltf.h>
#include <tuple>
#include <vector>

namespace cs::graphics::internal {

/// Converts GLTF primitives to OpenGL primitives.
// constexpr int toGLprimitiveMode(tinygltf::Primitive const& primitive)
inline int toGLprimitiveMode(tinygltf::Primitive const& primitive) {
  if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
    return GL_TRIANGLES;
  } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
    return GL_TRIANGLE_STRIP;
  } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN) {
    return GL_TRIANGLE_FAN;
  } else if (primitive.mode == TINYGLTF_MODE_POINTS) {
    return GL_POINTS;
  } else if (primitive.mode == TINYGLTF_MODE_LINE) {
    return GL_LINES;
  } else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP) {
    return GL_LINE_LOOP;
  } else {
    assert(0);
  }

  return 0;
}

/// Returns the parameter with the given name from the material.
template <typename T>
auto find_material_parameter(
    tinygltf::Material const& material, std::string const& name, T const& def) -> T {
  auto it = material.values.find(name);

  bool found = it != material.values.end();

  if (!found) {
    it    = material.additionalValues.find(name);
    found = it != material.additionalValues.end();
  }

  if (!found) {
    return def;
  } else {
    auto const& parameter = it->second;
    T           value{};
    for (typename T::length_type i = 0;
         i < std::min(value.length(),
                 static_cast<typename T::length_type>(parameter.number_array.size()));
         ++i) {
      value[i] = static_cast<typename T::value_type>(parameter.number_array[i]);
    }
    return value;
  }
}

/// Returns the parameter with the given name from the material.
template <>
auto find_material_parameter(
    tinygltf::Material const& material, std::string const& name, float const& def) -> float;

/// Returns the id of the texture with the given name from the material.
int find_texture_index(tinygltf::Material const& material, std::string const& name);

/// Loads a tinygltf::Image from the given filepath.
tinygltf::Image loadImage(std::string const& filepath);
} // namespace cs::graphics::internal
#endif // CS_GRAPHICS_TINY_GLTF_HELPER
