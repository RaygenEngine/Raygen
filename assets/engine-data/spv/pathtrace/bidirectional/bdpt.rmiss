#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

#include "sky.glsl"

struct LightPathVertex {
	vec3 position;
	vec3 normal;
	vec3 value; 
};

struct hitPayload
{
	vec3 radiance; // to be filled

	vec3 origin; // this ray stuff
	vec3 direction;
	vec3 attenuation; 

	int hitType; 
	uint seed;
	
	uint lightPathDepth;
	LightPathVertex lightpath[4];
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	prd.radiance = GetSkyColor(prd.origin, prd.direction);
	prd.hitType = 2; 
}
