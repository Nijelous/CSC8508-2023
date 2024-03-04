#version 460 core

#extension GL_ARB_bindless_texture : require

layout(std140, binding = 6) uniform TextureHandles {
	int handles[64];
	int index[6];
} texHandles;

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