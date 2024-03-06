#version 460 core

#extension GL_ARB_bindless_texture : require

layout(std140, binding = 6) uniform TextureHandles {
	sampler2D handles[64];
} texHandles;

layout(std140, binding = 7) uniform TextureHandleIDs{
	int albedoIndex;
	int normalIndex;
	int depthIndex;
	int shadowIndex;
	int albedoLightIndex;
	int specLightIndex;
} texIndices;


in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void)
{
	vec3 diffuse = texture(texHandles.handles[texIndices.albedoIndex], IN.texCoord).xyz;
	vec3 light = texture(texHandles.handles[texIndices.albedoLightIndex], IN.texCoord).xyz;
	vec3 specular = texture(texHandles.handles[texIndices.specLightIndex], IN.texCoord).xyz;

	fragColor.xyz = diffuse * 0.1;
	fragColor.xyz += diffuse * light;
	fragColor.xyz += specular;
	fragColor.a = 1.0;
}