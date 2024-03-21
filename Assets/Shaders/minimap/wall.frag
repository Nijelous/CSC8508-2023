#version 400 core

out vec4 fragColor;
in vec2 vsPosition;

uniform vec3 color;
uniform float viewRadius;
uniform vec2 mPlayerLocation;

void main(void) {
    if (length(vsPosition - mPlayerLocation) > viewRadius) discard;
    fragColor = vec4(color, 1);
}