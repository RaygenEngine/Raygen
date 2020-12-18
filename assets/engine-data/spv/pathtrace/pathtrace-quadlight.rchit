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

	vec3 attenuation;  // attenuation of THIS ray
	float nol;
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

	if(dot(ql.normal, -gl_WorldRayDirectionEXT) < 0) {
		prd.radiance = vec3(0); 
		prd.hitType = 2;
		return;
	}

	// direct hit or mirror
	if(prd.hitType == 0 || prd.hitType == 4) {
		prd.radiance = ql.color * ql.intensity;  
		prd.radiance = vec3(max(prd.radiance.x, 0.0),
		                    max(prd.radiance.y, 0.0),
							max(prd.radiance.z, 0.0)) / vec3(max(max(prd.radiance), 1.0));
		prd.hitType = 2;
		return;
	}

	vec3 hitpoint = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
	
	float dist = distance(hitpoint, gl_WorldRayOriginEXT);


	float attenuation = (ql.constantTerm + ql.linearTerm * dist + 
  			     ql.quadraticTerm * (dist * dist));

	float pdf_light = (attenuation) / (ql.width * ql.height * prd.nol); 
	float pdf_brdf = 1.0 / prd.sampleWeight;
	float mis_weight = 1.0 / (pdf_light + pdf_brdf);

	vec3 Le = ql.color * ql.intensity; 

	prd.radiance = Le;
	prd.sampleWeight = mis_weight;
	prd.hitType = 2;
}
