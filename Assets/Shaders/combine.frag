#version 400 core

uniform sampler2D 	albedoTex;
uniform sampler2D 	albedoLight;
uniform sampler2D 	specularLight;

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void)
{
	vec3 diffuse = texture(albedoTex, IN.texCoord).xyz;
	vec3 light = texture(albedoLight, IN.texCoord).xyz;
	vec3 specular = texture(specularLight, IN.texCoord).xyz;

	fragColor.xyz = diffuse * 0.1;
	fragColor.xyz += diffuse * light;
	fragColor.xyz += specular;
	fragColor.a = 1.0;
}