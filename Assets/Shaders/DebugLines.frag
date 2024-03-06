#version 460 core

#extension GL_ARB_bindless_texture : require

in Vertex
{
	vec4 colour;
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void)
{

	fragColor = IN.colour;

}