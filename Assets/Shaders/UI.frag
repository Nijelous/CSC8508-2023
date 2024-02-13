#version 400 core

uniform sampler2D iconTex;
uniform bool isOn;

in Vertex
{
    vec2 texCoord;
}IN;

out vec4 fragColor;

void main(void)
{ 
     if(isOn){
          fragColor = texture(iconTex, IN.texCoord);
     }
     else{
          fragColor = texture(iconTex, IN.texCoord) * vec4(0.06, 0.06, 0.06, 0.2);
     }
}