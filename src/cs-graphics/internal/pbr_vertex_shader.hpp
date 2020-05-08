////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PBR_VERTEX_SHADER_HPP
#define CS_GRAPHICS_PBR_VERTEX_SHADER_HPP

namespace cs::graphics::internal {

const char* GLTF_VERT = R"(
layout (location=0) in vec4 a_Position;
layout (location=1) in vec3 a_Normal;
layout (location=2) in vec4 a_Tangent;
layout (location=3) in vec2 a_TexCoord0;
layout (location=4) in vec2 a_TexCoord1;
layout (location=5) in vec2 a_Color0;
layout (location=6) in vec2 a_Joints0;
layout (location=7) in vec2 a_Weights0;

uniform mat4 u_MVPMatrix;

uniform mat4 u_ModelMatrix;
//uniform mat4 u_ViewMatrix;
//uniform mat4 u_ProjectionMatrix;
uniform mat3 u_NormalMatrix;

out vec3 v_Position;
out vec2 v_UV;

#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
out mat3 v_TBN;
#else
out vec3 v_Normal;
#endif
#endif

void main()
{
  vec4 pos = u_ModelMatrix * a_Position;
  v_Position = vec3(pos.xyz) / pos.w;

  #ifdef HAS_NORMALS
  #ifdef HAS_TANGENTS
  vec3 normalW = normalize(u_NormalMatrix * a_Normal);
  vec3 tangentW = normalize(u_NormalMatrix * a_Tangent.xyz);
  vec3 bitangentW = cross(normalW, tangentW) * a_Tangent.w;
  v_TBN = mat3(tangentW, bitangentW, normalW);
  #else // HAS_TANGENTS != 1
  v_Normal = normalize(u_NormalMatrix * a_Normal);
  #endif
  #endif

  #ifdef HAS_UV
  v_UV = a_TexCoord0;
  #else
  v_UV = vec2(0.0);
  #endif

  gl_Position = u_MVPMatrix * a_Position; // needs w for proper perspective correction
}

)";
}
#endif // CS_GRAPHICS_PBR_VERTEX_SHADER_HPP
