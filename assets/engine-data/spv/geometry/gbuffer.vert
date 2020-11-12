#version 460
#extension GL_GOOGLE_include_directive : enable
#include "global.glsl"

// out

layout(location=0) out Data
{ 
	vec2 uv;
	mat3 TBN;
	vec3 fragPos;
	vec4 clipPos; 
	vec4 prevClipPos;
	float drawIndex;
};

// in

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 textCoord;

// uniforms

layout(push_constant) uniform PC {
	mat4 modelMat;
	mat4 normalMat;
	mat4 mvpPrev;
	float drawIndex;
} push;

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };

void main() {
	vec4 posWCS = push.modelMat * vec4(position, 1.0);
	gl_Position = cam.viewProj * posWCS;
	clipPos = gl_Position;
	prevClipPos = push.mvpPrev * vec4(position, 1.0);
	drawIndex = push.drawIndex;

	

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

