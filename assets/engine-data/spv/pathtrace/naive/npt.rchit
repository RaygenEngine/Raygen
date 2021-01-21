#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable

// TODO:
#define RAY
#include "global.glsl"

#include "pathtrace/gltf-surface.glsl"

struct hitPayload
{
	vec3 radiance; // to be filled

	vec3 origin; // this ray stuff
	vec3 direction;
	vec3 attenuation; 

	int hitType; 
	uint seed;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	
	Surface surface = surfaceFromGeometryGroup();

	prd.radiance = surface.emissive;

	bool isSpecialPath;
	float pdf;
	if(!sampleBSDF(surface, prd.attenuation, pdf, isSpecialPath, prd.seed)) {
		prd.attenuation = vec3(0);
		prd.hitType = 2;
		return;
	}

	// bsdf * nol / pdf
	prd.attenuation = prd.attenuation * absNdot(surface.o) / pdf;
	prd.hitType = 1; // general
	prd.origin = surface.position;
	prd.direction = getOutgoingDir(surface);	
}
