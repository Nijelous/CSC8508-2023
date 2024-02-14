#version 400 core

//uniform vec4 		objectColour;
//uniform sampler2D 	mainTex;
//uniform sampler2D   normTex;
uniform sampler2D 	diffuseTex;
//uniform sampler2DShadow shadowTex;

//uniform vec3	lightPos;
//uniform float	lightRadius;
//uniform vec4	lightColour;

//uniform vec3	cameraPos;

//uniform bool hasTexture;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void)
{
	fragColour = texture(diffuseTex, IN.texCoord);
	fragColour.a = 1;
}