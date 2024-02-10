#version 400 core

uniform sampler2D iconTex;

in Vertex
{
    vec2 texCoord;
}IN;

out vec4 fragColor;

void main(void)
{ 
     fragColor = texture(iconTex, IN.texCoord);
}