#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

#include "sky.glsl"

struct hitPayload
{
	vec3 radiance; // previous radiance

	vec3 origin; // this ray stuff
	vec3 direction;
	vec3 attenuation; 
	float pdf;

	int hitType; 
	uint seed;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	prd.radiance = GetSkyColor(gl_WorldRayOriginEXT, gl_WorldRayDirectionEXT);
	prd.hitType = 3; 
}
