#version 460 core

#extension GL_ARB_bindless_texture : require

uniform samplerCube cubeTex;

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

in Vertex {
	vec3 viewDir;
} IN;

out vec4 fragColour;

void main(void)	{
	vec4 samp = texture(cubeTex,normalize(IN.viewDir));
	fragColour = pow(samp, vec4(2.2f));
}