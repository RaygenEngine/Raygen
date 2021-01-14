#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
// TODO:
#define RAY
#include "global.glsl"

struct hitPayload
{
	vec3 radiance; // previous radiance

	vec3 origin; // stuff of THIS ray
	vec3 direction;
	vec3 attenuation; 
	float weightedPdf;

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

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 7, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {

	int quadId = gl_InstanceCustomIndexEXT;
	Quadlight ql = quadlights.light[quadId];

	float LnoL = dot(ql.normal, -gl_WorldRayDirectionEXT);

	// behind
	if(LnoL < BIAS) {
		prd.radiance = vec3(0);
		prd.hitType = 4;
		return;	
	}

	// direct hit
	if(prd.hitType == 0) {
		prd.radiance = ql.color;
		prd.hitType = 4; // TODO: continue path, tho analytic light surface has no actual material values
		return;	
	}

	// from mirror
	if(prd.hitType == 2) {
		prd.radiance = ql.color * ql.intensity;
		prd.hitType = 4;
		return;	
	}

	LnoL = abs(LnoL);

	vec3 hitpoint = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
	float dist = distance(hitpoint, gl_WorldRayOriginEXT);
	float pdf_pickLight = 1.0 / float(quadlightCount);
	float pdf_lightArea = (dist * dist) / (ql.width * ql.height * LnoL);
	float pdf_light = pdf_pickLight * pdf_lightArea; 

	prd.radiance = ql.color * ql.intensity;  
	prd.weightedPdf = pdf_light + prd.weightedPdf; // prev prd.weightedPdf = pdf_bsdf
	prd.hitType = 3;
}
