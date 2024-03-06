#version 460 core

#extension GL_ARB_bindless_texture : require

layout(std140, binding = 1) uniform StaticBlock{
	mat4 orthProj;
	vec2 pixelSize;
} staticData;

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

out vec4 fragColour;

in Vertex
{
	vec2 texCoord;
} IN;

void main(void) {
	vec2 coord = vec2(gl_FragCoord.xy * staticData.pixelSize);
	float depth = texture(texHandles.handles[texIndices.depthIndex], coord.xy).r;
	if(gl_FragCoord.z <= depth + 0.001f) discard;
	fragColour = vec4(1,0,0,1);
}