#version 420 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 shadowMatrix 	= mat4(1.0f);

layout(location = 0) in vec3 position;

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjView;
	vec3 camPos;
} camData;

uniform float lightRadius = 1.0f;
uniform vec3 lightPos;
uniform vec4 lightColour;

void main() {
	vec3 scale = vec3(lightRadius, lightRadius, lightRadius);
	vec3 worldPos = (position * scale) + lightPos;
	gl_Position = (camData.projMatrix * camData.viewMatrix) * vec4(worldPos, 1.0);
}