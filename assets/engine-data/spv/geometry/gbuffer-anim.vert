#version 450

// out

layout(location=0) out Data
{ 
	vec2 uv;
	mat3 TBN;
	vec3 fragPos;
};

// in

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 textCoord;
layout(location = 4) in ivec4 joint;
layout(location = 5) in vec4 weight;

// uniforms

layout(push_constant) uniform PC {
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

layout(std430, set = 2, binding = 0) readonly buffer SSBO_Joints {
	mat4 invBindMatrix[];
} jm;

void main() {
	mat4 skinMat = 
		weight.x * jm.invBindMatrix[joint.x] +
		weight.y * jm.invBindMatrix[joint.y] +
		weight.z * jm.invBindMatrix[joint.z] +
		weight.w * jm.invBindMatrix[joint.w];


	vec4 posWCS = push.modelMat * skinMat * vec4(position, 1.0);
	gl_Position = camera.viewProj * posWCS;
	fragPos = posWCS.xyz;

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
                                      

