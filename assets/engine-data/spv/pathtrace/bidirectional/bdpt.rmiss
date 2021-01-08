#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

#include "sky.glsl"

struct hitPayload
{
	vec3 origin; 
	vec3 direction;
	vec3 normal;
	vec3 throughput;

	int hitType; 
	uint seed;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	prd.throughput = GetSkyColor(prd.origin, prd.direction);
	prd.hitType = 2; 
}
