#version 420 core

uniform sampler2D iconTex;
uniform float uTransparency;


layout(std140, binding = 5) uniform IconBlock {
	bool useTexture;
    float uTransparency;
} iconData; 


in Vertex
{
    vec2 texCoord;
}IN;

out vec4 fragColor;

void main(void)
{ 
    vec4 texColor = texture(iconTex, IN.texCoord);
    fragColor = vec4(texColor.rgb, texColor.a * uTransparency);

}