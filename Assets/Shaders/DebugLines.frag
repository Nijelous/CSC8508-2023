#version 420 core

uniform sampler2D 	mainTex;

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