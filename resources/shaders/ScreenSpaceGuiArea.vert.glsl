#version 330

vec2 positions[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5),
    vec2(-0.5,  0.5),
    vec2( 0.5,  0.5)
);

uniform vec2 iPosition;
uniform vec2 iScale;

out vec2 vTexCoords;
out vec4 vPosition;

void main() {
    vec2 p = positions[gl_VertexID];
    vTexCoords = vec2(p.x, -p.y) + 0.5;
    vPosition = vec4((p * iScale + iPosition) * 2 - 1, 0, 1);
    gl_Position = vPosition;
}         