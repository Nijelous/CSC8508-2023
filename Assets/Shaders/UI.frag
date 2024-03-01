#version 420 core

uniform sampler2D iconTex;

layout(std140, binding = 5) uniform IconBlock {
	bool isOn;
	bool useTexture;
} iconData; 

in Vertex
{
    vec2 texCoord;
}IN;

out vec4 fragColor;

void main(void)
{ 
     if(iconData.isOn){
          fragColor = texture(iconTex, IN.texCoord);
     }
     else{
          fragColor = texture(iconTex, IN.texCoord) * vec4(0.06, 0.06, 0.06, 0.2);
     }
}