#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout(location=0) out Data
{ 
	vec3 fragPos; 
	vec2 uv[2];
	mat3 TBN;
};

// in

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv0;
layout(location = 5) in vec2 uv1;

// uniforms

layout(push_constant) uniform ModelData {
	mat4 modelMat;
	//mat4 normalMat;
} push;

layout(binding = 0) uniform UBO_Globals {
	mat4 viewProj;
} globals;

void main() {
	gl_Position = globals.viewProj * push.modelMat * vec4(pos, 1.0);

	fragPos = vec3(push.modelMat * vec4(pos,1));
	uv[0] = uv0; 
	uv[1] = uv1; 
	
	// TODO: normal matrix
	vec3 T = normalize(vec3(push.modelMat) * tangent);
	vec3 B = normalize(vec3(push.modelMat) * bitangent);
	vec3 N = normalize(vec3(push.modelMat) * normal);
	
    TBN = mat3(T, B, N);
}