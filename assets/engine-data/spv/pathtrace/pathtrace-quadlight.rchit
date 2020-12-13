#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
// TODO:
#define RAY
#include "global.glsl"

struct hitPayload
{
	vec3 radiance;
	vec3 attenuation;

	float sampleWeight;

	vec3 origin;
	vec3 direction;

	int done;
	int depth;
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

	if(prd.depth != 1){
		prd.radiance = ql.color; // final computation will be : Le * Brdf * cosTheta
		prd.done = 1;
		return;
	}

	float p_selectLight = 1.0 / float(quadlightCount);
	float pdf_area = 1.0 / (ql.width * ql.height);
	float pdf_light = pdf_area * p_selectLight;
	float pdf_brdf = 1.0 / prd.sampleWeight;

	vec3 hitpoint = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
	
	float dist = distance(hitpoint, gl_WorldRayOriginEXT);
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
  			     ql.quadraticTerm * (dist * dist));

	vec3 Le = ql.color * ql.intensity * attenuation; 

	prd.sampleWeight = 1.0  / (pdf_light + pdf_brdf);
	prd.radiance = Le; // final computation will be : Le * Brdf * cosTheta * mis_weight
	prd.done = 1;
}
