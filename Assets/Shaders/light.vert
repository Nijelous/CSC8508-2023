#version 460 core

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 position;

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjView;
	vec3 camPos;
} camData;

layout(std140, binding = 2) uniform LightBlock {
	vec3 lightDirection;
	float minDotProd;
	vec3 lightPos;
	float dimDotProd;
	vec3 lightColour;	
	float lightRadius;	
} lightData;

void main() {
	vec3 scale = vec3(lightData.lightRadius, lightData.lightRadius, lightData.lightRadius);
	vec3 worldPos = (position * scale) + lightData.lightPos;
	gl_Position = (camData.projMatrix * camData.viewMatrix) * vec4(worldPos, 1.0);
}