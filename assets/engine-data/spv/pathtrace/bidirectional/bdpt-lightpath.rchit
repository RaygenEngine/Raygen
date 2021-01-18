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
	vec3 origin; 
	vec3 direction;
	vec3 normal;
	vec3 throughput;

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

layout(location = 1) rayPayloadInEXT hitPayload prd;

void main() {
	
	Surface surface = surfaceFromGeometryGroup();

	bool isDiffusePath, isRefractedPath;
	float pdf_path, pdf_bsdf;
	if(!sampleBSDF(surface, prd.throughput, pdf_path, pdf_bsdf, isDiffusePath, isRefractedPath, prd.seed)) {
		prd.throughput = vec3(0);
		prd.hitType = 1;
		return;
	}

	float pdf = pdf_path * pdf_bsdf;

	// projection solid angle form
	pdf /= absNdot(surface.o);

	prd.normal = surface.basis.normal;
	prd.throughput /= pdf;
	prd.hitType = 0; // continue
	prd.origin = surface.position;
	prd.direction = getOutgoingDir(surface);
}
