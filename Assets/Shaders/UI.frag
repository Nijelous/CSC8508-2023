#version 460 core

#extension GL_ARB_bindless_texture : require



layout(std140, binding = 5) uniform IconData {
	float transparency;
} iconBlock;

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
}IN;

out vec4 fragColor;

void main(void)
{ 
    vec4 texColor = texture(texHandles.handles[texIndices.albedoIndex], IN.texCoord);
    fragColor = vec4(texColor.rgb, texColor.a * iconBlock.transparency);
}