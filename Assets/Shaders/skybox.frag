#version 460 core

#extension GL_ARB_bindless_texture : require

uniform samplerCube cubeTex;

layout(std140, binding = 6) uniform TextureHandles {
	int handles[64];
	int index[6];
} texHandles;

in Vertex {
	vec3 viewDir;
} IN;

out vec4 fragColour;

void main(void)	{
	vec4 samp = texture(cubeTex,normalize(IN.viewDir));
	fragColour = pow(samp, vec4(2.2f));
}