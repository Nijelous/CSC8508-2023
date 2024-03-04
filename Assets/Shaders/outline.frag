#version 420 core

uniform sampler2D depthTex;

layout(std140, binding = 1) uniform StaticBlock{
	mat4 orthProj;
	vec2 pixelSize;
} staticData;

out vec4 fragColour;

in Vertex
{
	vec2 texCoord;
} IN;

void main(void) {
	vec2 coord = vec2(gl_FragCoord.xy * staticData.pixelSize);
	float depth = texture(depthTex, coord.xy).r;
	if(gl_FragCoord.z <= depth + 0.001f) discard;
	fragColour = vec4(1,0,0,1);
}