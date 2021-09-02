#include "pathtrace/gltf-surface.glsl"
#include "pathtrace/lights.glsl"

struct hitPayload
{
	vec3 radiance; // previous radiance

	vec3 origin; // stuff of THIS ray
	vec3 direction;
	vec3 attenuation;

	int hitType; // previous hit type
	uint seed;
};

layout(push_constant) uniform PC
{
	int iteration;
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

	// DIRECT // SMATH: when we'll use a non uniform pdf for picking, use it for delta lights too

	vec3 radiance = vec3(0.f);
	
	if(quadlightCount > 0 && surface.a >= SPEC_THRESHOLD) {
		int totalLights = quadlightCount; 
		float u = rand(prd.seed);
		int qIndex = int(floor(u * totalLights));
		float pdf_pickLight = 1.0 / float(totalLights); 

		Quadlight ql = quadlights.light[qIndex];
		radiance += Quadlight_EstimateDirect(topLevelAs, ql, qIndex, surface, prd.seed) / pdf_pickLight;
	}
	
	for(int i = 0; i < pointlightCount; ++i) {
		Pointlight pl = pointlights.light[i];
		radiance += Pointlight_EstimateDirect(topLevelAs, pl, surface);
	}

	for(int i = 0; i < spotlightCount; ++i) {
		Spotlight sl = spotlights.light[i];
		radiance += Spotlight_EstimateDirect(topLevelAs, sl, surface); 
	}

	for(int i = 0; i < dirlightCount; ++i) {
		Dirlight dl = dirlights.light[i];
		radiance += Dirlight_EstimateDirect(topLevelAs, dl, surface); 
	}

	radiance += surface.emissive; // works like naive approach
	prd.radiance = radiance;

	// INDIRECT

	bool isSpecialPath;
	float pdf;
	if(!sampleBSDF(surface, prd.attenuation, pdf, isSpecialPath, prd.seed)) {
		prd.attenuation = vec3(0);
		prd.hitType = 3;
		return;
	}

	// bsdf * nol / pdf
	prd.attenuation = prd.attenuation * absNdot(surface.o) / pdf;
	prd.hitType = !isSpecialPath ? 1 : 2; 
	prd.origin = surface.position;
	prd.direction = getOutgoingDir(surface);	
}
