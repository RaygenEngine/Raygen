#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv0;
layout(location = 5) in vec2 uv1;

layout(push_constant) uniform ModelData {
	mat4 modelMat;
	mat4 normalMat;
} push;

layout(binding = 0) uniform UniformBufferObject {
	mat4 viewProj;
} ubo;

out Data
{ 
	vec3 fragPos; 
	vec2 uv[2];
	mat3 TBN;
} OUT;

void main() {
	gl_Position = ubo.viewProj * push.modelMat * vec4(pos, 1.0);

	OUT.fragPos = vec3(modelMat * vec4(pos,1));
	OUT.uv[0] = uv0; 
	OUT.uv[1] = uv1; 
	
	vec3 T = normalize(normalMat * tangent);
	vec3 B = normalize(normalMat * bitangent);
	vec3 N = normalize(normalMat * normal);
	
    OUT.TBN = mat3(T, B, N);
}