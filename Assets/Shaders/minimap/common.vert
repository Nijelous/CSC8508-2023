#version 330 core

layout(location=0)in vec2 inPosition;
layout(location=1)in vec2 inTexcoord;
out vec2 vsTexcoord;
out vec2 vsPosition;
uniform mat3 M;

void main(void) {
    vsPosition = inPosition;
    vsTexcoord = inTexcoord;
    vec2 pos = (M * vec3(inPosition, 1)).xy;
    gl_Position = vec4(pos, 0, 1.0);
}
