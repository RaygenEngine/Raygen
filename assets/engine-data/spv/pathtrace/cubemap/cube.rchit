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
#include "pathtrace/lights.glsl"

struct hitPayload {
	vec3 radiance; // previous radiance

	vec3 origin; // stuff of THIS ray
	vec3 direction;
	vec3 attenuation;

	int hitType; // previous hit type
	uint seed;
};

layout(push_constant) uniform PC {
	int samples;
	int bounces;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int quadlightCount;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAs;
layout(set = 4, binding = 0, std430) readonly buffer Pointlights { Pointlight light[]; } pointlights;
layout(set = 5, binding = 0, std430) readonly buffer Spotlights { Spotlight light[]; } spotlights;
layout(set = 6, binding = 0, std430) readonly buffer Dirlights { Dirlight light[]; } dirlights;
layout(set = 7, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {

	Surface surface = surfaceFromGeometryGroup();

	vec3 radiance = vec3(0.f);

	bool isDiffusePath, isRefractedPath;
	float pdf_path, pdf_bsdf;
	if(!sampleBSDF(surface, prd.attenuation, pdf_path, pdf_bsdf, isDiffusePath, isRefractedPath, prd.seed)) {
		prd.attenuation = vec3(0);
		prd.hitType = 3;
		return;
	}

	bool isDeltaPath = surface.a < SPEC_THRESHOLD && !isDiffusePath;

	// DIRECT 
	if(!isDeltaPath) {
		int totalLights = pointlightCount + quadlightCount + spotlightCount + dirlightCount;
		float u = rand(prd.seed);
		int i = int(floor(u * totalLights));
		float pdf_pickLight = 1.0 / float(totalLights); // pick one of the lights

#define pIndex i
#define qIndex pIndex - pointlightCount
#define sIndex qIndex - quadlightCount
#define dIndex sIndex - spotlightCount

		if(pIndex < pointlightCount) {
			Pointlight pl = pointlights.light[pIndex];
			radiance += Pointlight_EstimateDirect(topLevelAs, pl, surface, isDiffusePath);
		}
		else if (qIndex < quadlightCount) {
			Quadlight ql = quadlights.light[qIndex];
			radiance += Quadlight_EstimateDirect(topLevelAs, ql, qIndex, surface, isDiffusePath, prd.seed); // TODO: there is something wrong with the probabilites resulting in brighter light than naive approach
		}
		else if (sIndex < spotlightCount) {
			Spotlight sl = spotlights.light[sIndex];
			radiance += Spotlight_EstimateDirect(topLevelAs, sl, surface, isDiffusePath); 
		}
		else if (dIndex < dirlightCount) {
			Dirlight dl = dirlights.light[dIndex];
			radiance += Dirlight_EstimateDirect(topLevelAs, dl, surface, isDiffusePath); 
		}

		radiance /= (pdf_pickLight * pdf_path); // CHECK: boost by chance of diffuse or glossy?
	}

	radiance += surface.emissive; // works like naive approach
	prd.radiance = radiance;

	// INDIRECT - next step
	{
		// bsdf * nol 
		prd.attenuation = prd.attenuation * absNdot(surface.o) / (pdf_path * pdf_bsdf);
		prd.hitType = isDeltaPath || isRefractedPath ? 2 : 1; // special or general
		prd.origin = surface.position;
		prd.direction = getOutgoingDir(surface);	
	}
}
