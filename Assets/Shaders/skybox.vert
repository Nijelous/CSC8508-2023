#version 460 core

#extension GL_ARB_bindless_texture : require


layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjMatrix;
	vec3 camPos;
} camData;

in  vec3 position;

out Vertex {
	vec3 viewDir;
} OUT;

void main(void)		{
	vec3 pos = position;
	mat4 invproj  = inverse(camData.projMatrix);
	pos.xy	  *= vec2(invproj[0][0],invproj[1][1]);
	pos.z 	= -1.0f;

	OUT.viewDir		= transpose(mat3(camData.viewMatrix)) * normalize(pos);
	gl_Position		= vec4(position, 1.0);
}
