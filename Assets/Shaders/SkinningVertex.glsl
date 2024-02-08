#version 400

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec4 jointWeights;
in ivec4 jointIndices;

uniform mat4 	joints[110];

//uniform mat4 	otherJoints[128];

uniform float frameLerp;

out Vertex {
	vec2 texCoord;
	//vec3 normal;
} OUT;

void main(void) {
	vec4 localNormal = vec4(normal, 1.0f);
	vec4 localPos 	= vec4(position, 1.0f);
	vec4 skelPos 	= vec4(0,0,0,0);
	vec4 otherSkelPos 	= vec4(0,0,0,0);

	//vec4 skelNormal = vec4(0,0,0,0);
	for(int i = 0; i < 4; ++i) {
		int   jointIndex 	= jointIndices[i];
		float jointWeight 	= jointWeights[i];

		skelPos += joints[jointIndex] * localPos * jointWeight;

		//skelNormal += joints[jointIndex] * localNormal * jointWeight;
	}
	//skelPos.xyz = position.xyz;


	//OUT.normal = mat3(modelMatrix) * normalize(skelNormal.xyz);

	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(skelPos.xyz, 1.0);
	OUT.texCoord = texCoord;

}