#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
// TODO:
#define RAY
#include "global.glsl"

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

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 7, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {

	int quadId = gl_InstanceCustomIndexEXT;
	Quadlight ql = quadlights.light[quadId];

	//if(prd.depth == 0){
	//	prd.radiance = ql.color; // WIP: final computation will be , hitType = mirror  and payload depth ??
	//	prd.done = 1;
	//	return;
	//}

	float p_selectLight = 1.0 / float(quadlightCount);
	float pdf_area = 1.0 / (ql.width * ql.height);
	float pdf_light = pdf_area * p_selectLight;
	float pdf_brdf = 1.0 / prd.sampleWeight;

	vec3 hitpoint = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
	
	float dist = distance(hitpoint, gl_WorldRayOriginEXT);
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
  			     ql.quadraticTerm * (dist * dist));

	vec3 Le = ql.color * ql.intensity; 

	prd.radiance = Le;
	prd.attenuation *= attenuation; // total atten
	prd.sampleWeight = 1.0  / (pdf_light + pdf_brdf);
	prd.hitType = 2;
}
