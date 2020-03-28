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
	mat4 viewProj;
	mat3 normalMat;
	float padding[9];
} push;

void main() {
	gl_Position = push.viewProj * push.modelMat * vec4(pos, 1.0);

	fragPos = vec3(push.modelMat * vec4(pos,1));
	uv[0] = uv0; 
	uv[1] = uv1; 
	
	vec3 T = normalize(push.normalMat * tangent.xyz);
	vec3 N = normalize(push.normalMat * normal.xyz);
	vec3 B = normalize(push.normalMat * bitangent.xyz);
	
    TBN = mat3(T, B, N);
}