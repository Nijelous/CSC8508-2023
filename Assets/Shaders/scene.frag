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
	vec4 shadowProj;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColor[2];

void main(void)
{
	float shadow = 1.0; // New !
	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	vec3 bumpNormal = texture(texHandles.handles[texIndices.normalIndex], IN.texCoord).rgb;
	bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));
	
	//Author: B Schwarz - if we put shadow-mapping back in, we will need this. But for now, let's can it. 
//	if( IN . shadowProj . w > 0.0) { // New !
//		shadow = textureProj ( texHandles.handles[texIndices.index[shadowIndex]] , IN . shadowProj ) * 0.5f;
//	}
	
	fragColor[0] = texture2D(texHandles.handles[texIndices.albedoIndex],IN.texCoord);
	fragColor[1] = vec4(bumpNormal.xyz * 0.5 + 0.5, 1.0);
	fragColor[0].a = 1.0;
	fragColor[1].a = 1.0;

}