#version 460 core

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;
layout(location = 7) in mat4 instanceMatrix;

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjMatrix;
	vec3 camPos;
} camData;

layout(std140, binding = 3) uniform ObjectBlock {
	mat4 modelMatrix;
	mat4 shadowMatrix;
	vec4 objectColour;
	bool hasVertexColours;
} objectData; 

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} OUT;

void main(void)
{
	mat4 mvp;
	mat3 normalMatrix;

	mvp 		  = (camData.projMatrix * camData.viewMatrix * objectData.modelMatrix);
	normalMatrix = transpose ( inverse ( mat3 (objectData.modelMatrix )));
	OUT.worldPos 	= ( objectData.modelMatrix * vec4 ( position ,1)). xyz ;

	vec3 wNormal = normalize ( normalMatrix * normalize ( normal ));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	OUT.shadowProj 	=  objectData.shadowMatrix * vec4 ( position,1);
	OUT.normal 		= wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;

	
	OUT.texCoord	= texCoord;
	OUT.colour		= objectData.objectColour;

	if(objectData.hasVertexColours) {
		OUT.colour		= objectData.objectColour * colour;
	}
	gl_Position		= mvp * vec4(position, 1.0);
}