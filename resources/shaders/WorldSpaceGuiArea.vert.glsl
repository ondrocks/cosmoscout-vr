#version 330

vec2 positions[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5)
);

uniform mat4 uMatModelView;
uniform mat4 uMatProjection;
uniform bool uUseLinearDepth;

out vec2 vTexCoords;
out vec4 vPosition;

void main() {
    vec2 p = positions[gl_VertexID];
    vTexCoords = vec2(p.x, -p.y) + 0.5;
    vPosition = uMatModelView * vec4(p, 0, 1);
    gl_Position = uMatProjection * vPosition;

    if (uUseLinearDepth) {
        gl_Position.z = 0;
    } else {
        if (gl_Position.w > 0) {
            gl_Position /= gl_Position.w;
            if (gl_Position.z >= 1) {
                gl_Position.z = 0.999999;
            }
        }
    }
}