#version 460 core

#extension GL_ARB_bindless_texture : require

uniform sampler2D 	mainTex;

layout(std140, binding = 6) uniform TextureHandles {
	int handles[64];
	int index[6];
} texHandles;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void)
{

	float alpha = texture(mainTex, IN.texCoord).r;
		
	if(alpha < 0.00001f) {
		discard;
	}
		
	fragColor = IN.colour * vec4(1,1,1,alpha);
	
}