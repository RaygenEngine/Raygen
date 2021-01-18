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
	
	Surface surface = surfaceFromGeometryGroup();

	prd.radiance = surface.emissive;

	bool isDiffusePath, isRefractedPath;
	float pdf_path, pdf_bsdf;
	if(!sampleBSDF(surface, prd.attenuation, pdf_path, pdf_bsdf, isDiffusePath, isRefractedPath, prd.seed)) {
		prd.attenuation = vec3(0);
		prd.hitType = 2;
		return;
	}

	float pdf = pdf_path * pdf_bsdf;

	// bsdf * nol / pdf
	prd.attenuation = prd.attenuation * absNdot(surface.o) / pdf;
	prd.hitType = 1; // general
	prd.origin = surface.position;
	prd.direction = getOutgoingDir(surface);	
}
