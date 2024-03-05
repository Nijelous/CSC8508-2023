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
	vec4 colour;
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void)
{

	float alpha = texture(texHandles.handles[texIndices.albedoIndex], IN.texCoord).r;
		
	if(alpha < 0.00001f) {
		discard;
	}
		
	fragColor = IN.colour * vec4(1,1,1,alpha);
	
}