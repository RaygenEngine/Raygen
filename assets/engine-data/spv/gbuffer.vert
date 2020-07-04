#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout(location=0) out Data
{ 
	vec3 fragPos; 
	vec2 uv;
	mat3 TBN;
};

// in

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 textCoord;

// uniforms

layout(push_constant) uniform ModelData {
	mat4 modelMat;
	mat4 normalMat;
} push;

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} camera;

void main() {
	gl_Position = camera.viewProj * push.modelMat * vec4(position, 1.0);

	fragPos = vec3(push.modelMat * vec4(position, 1.0));
	uv = textCoord;


	vec3 T = normalize(mat3(push.normalMat) * tangent);
   	vec3 N = normalize(mat3(push.normalMat) * normal);

	// Gram-Schmidt process + cross product
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);

	TBN = mat3(T, B, N); 
}                                       

