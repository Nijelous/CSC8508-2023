#version 460 core

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(std140, binding = 1) uniform StaticBlock{
	mat4 orthProj;
	vec2 pixelSize;
} staticData;

out Vertex
{
	vec2 texCoord;
} OUT;

void main(void)
{
	gl_Position		= staticData.orthProj * vec4(position, 1.0);
	OUT.texCoord	= texCoord;

}