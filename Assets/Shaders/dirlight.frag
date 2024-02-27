#version 420 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform sampler2D 	depthTex;
uniform sampler2D normTex;

uniform vec2 pixelSize;
uniform vec3 lightDirection;
uniform vec3 lightPos;
uniform float lightRadius;
uniform vec4	lightColour;

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjView;
	vec3 camPos;
} camData;

layout(std140, binding = 1) uniform StaticBlock {
	mat4 orthProj;
	vec2 pixelSize;
} staticData;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main(void)
{
	vec2 texCoord = vec2(gl_FragCoord.xy * staticData.pixelSize);
	float depth = texture(depthTex, texCoord.xy).r;
	vec3 ndcPos = vec3(texCoord, depth) * 2.0 - 1.0;
	vec4 invClipPos = camData.invProjView * vec4(ndcPos, 1.0);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	float dist = length(lightPos - worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

	if(atten == 0.0) { discard; }


	vec3 normal = normalize(texture(normTex, texCoord.xy).xyz * 2.0 - 1.0);
	vec3 incident = normalize(-lightDirection);
	vec3 viewDir = normalize(camData.camPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	float lambert = clamp(dot(incident, normal), 0.0, 1.0);
	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0);
	diffuseOutput = vec4(lightColour.xyz * lambert, 1.0);

	specularOutput = vec4(lightColour.xyz * specFactor * 0.33, 1.0);

}