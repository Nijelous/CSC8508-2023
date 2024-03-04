#version 460 core

#extension GL_ARB_bindless_texture : require

uniform sampler2D 	albedoTex;
uniform sampler2D 	albedoLight;
uniform sampler2D 	specularLight;

layout(std140, binding = 6) uniform TextureHandles {
	int handles[64];
	int index[6];
} texHandles;


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