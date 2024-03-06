#version 460 core

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 5) in vec4 jointWeights;
layout(location = 6) in ivec4 jointIndices;

out Vertex
{
	vec2 texCoord;
} OUT;

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjView;
	vec3 camPos;

} camData;

layout(std140, binding = 3) uniform ObjectBlock {
	mat4 modelMatrix;
	mat4 shadowMatrix;
	vec4 objectColour;
	bool hasVertexColours;
} objectData;




void main(void)
{
	mat4 mvp 		  = (camData.projMatrix * camData.viewMatrix * objectData.modelMatrix);
	vec4 localPos 	= vec4(position, 1.0f);
	OUT.texCoord = texCoord;
	gl_Position = mvp * localPos;
}


