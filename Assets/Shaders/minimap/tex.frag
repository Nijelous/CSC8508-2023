#version 400 core

out vec4 fragColor;
in vec2 vsTexcoord;

uniform sampler2D tex;

void main(void) { fragColor = texture(tex, vsTexcoord); }