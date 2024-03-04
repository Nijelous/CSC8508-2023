#version 460 core

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;

layout(std140, binding = 6) uniform TextureHandles {
	int handles[64];
	int index[6];
} texHandles;

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjView;
	vec3 camPos;
} camData;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
} OUT;

void main(void)
{
	gl_Position		= camData.projMatrix * camData.viewMatrix * vec4(position, 1.0);
	OUT.texCoord	= texCoord;
	OUT.colour		= colour;
}