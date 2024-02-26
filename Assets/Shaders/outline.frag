#version 400 core

uniform sampler2D depthTex;
uniform vec2 pixelSize;

out vec4 fragColour;

in Vertex
{
	vec2 texCoord;
} IN;

void main(void) {
	vec2 coord = vec2(gl_FragCoord.xy * pixelSize);
	float depth = texture(depthTex, coord.xy).r;
	if(gl_FragCoord.z <= depth + 0.001f) discard;
	fragColour = vec4(1,0,0,1);
}