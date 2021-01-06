#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
// TODO:
#define RAY
#include "global.glsl"

struct hitPayload
{
	vec3 origin; 
	vec3 direction;
	vec3 normal;
	vec3 throughput;

	int hitType; 
	uint seed;

	// WIP:
	vec3 albedo;
	vec3 f0;
	float opacity;
	float a;
};

layout(push_constant) uniform PC
{
	int bounces;
	int frame;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int quadlightCount;
};

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 7, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {

	int quadId = gl_InstanceCustomIndexEXT;
	Quadlight ql = quadlights.light[quadId];

	float LnoL = dot(ql.normal, -gl_WorldRayDirectionEXT);

	if (LnoL < BIAS) {
		prd.throughput = vec3(0); 
		prd.hitType = 2;
		return;
	}

	prd.throughput = ql.color * ql.intensity;  

	// direct hit 
	if(prd.hitType == 0) {
		prd.throughput = ql.color;
	}

	prd.hitType = 2;
}
