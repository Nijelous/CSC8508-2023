#version 460 core

#extension GL_ARB_bindless_texture : require

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjView;
	vec3 camPos;
} camData;

layout(std140, binding = 1) uniform StaticBlock{
	mat4 orthProj;
	vec2 pixelSize;
} staticData;

layout(std140, binding = 2) uniform LightBlock {
	vec3 lightDirection;
	float minDotProd;
	vec3 lightPos;
	float dimDotProd;
	vec3 lightColour;	
	float lightRadius;	
} lightData;

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

out vec4 diffuseOutput;
out vec4 specularOutput;

void main(void)
{
	vec2 texCoord = vec2(gl_FragCoord.xy * staticData.pixelSize);
	float depth = texture(texHandles.handles[texIndices.depthIndex], texCoord.xy).r;
	vec3 ndcPos = vec3(texCoord, depth) * 2.0 - 1.0;
	vec4 invClipPos = camData.invProjView * vec4(ndcPos, 1.0);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	float dist = length(lightData.lightPos - worldPos);
	float atten = 1.0 - clamp(dist / lightData.lightRadius, 0.0, 1.0);

	if(atten == 0.0) { discard; }

	vec3 normal = normalize(texture(texHandles.handles[texIndices.normalIndex], texCoord.xy).xyz * 2.0 - 1.0);
	vec3 incident = normalize(lightData.lightPos - worldPos);
	vec3 viewDir = normalize(camData.camPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	float lambert = clamp(dot(incident, normal), 0.0, 1.0);
	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0);
	vec3 attenuated = lightData.lightColour * atten;
	diffuseOutput = vec4(attenuated * lambert, 1.0);

	specularOutput = vec4(attenuated * specFactor * 0.33, 1.0);

}