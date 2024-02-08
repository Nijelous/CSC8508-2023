#version 400 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);
uniform mat4 shadowMatrix 	= mat4(1.0f);

layout(location = 0) in vec3 position;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;

void main() {
	vec3 scale = vec3(lightRadius, lightRadius, lightRadius);
	vec3 worldPos = (position * scale) + lightPos;
	gl_Position = (projMatrix * viewMatrix) * vec4(worldPos, 1.0);
}