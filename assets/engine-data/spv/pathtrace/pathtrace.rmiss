#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

#include "sky.glsl"

// Values before being filled
struct hitPayload
{
	vec3 radiance; // previous radiance

	vec3 origin; // origin and dir of THIS ray
	vec3 direction;

	vec3 attenuation; // attenuation and weight of THIS ray
	float sampleWeight;

	int hitType; // previous hit type
	uint seed;
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

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	prd.radiance = GetSkyColor(gl_WorldRayOriginEXT, gl_WorldRayDirectionEXT);
	prd.hitType = 3; 
}
