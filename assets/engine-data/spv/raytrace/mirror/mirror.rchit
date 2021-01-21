#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_query: require
// TODO:
#define RAY
#include "global.glsl"

#include "global-descset.glsl"
#include "pathtrace/gltf-surface.glsl"
#include "radiance.glsl"


struct hitPayload
{
	vec3 radiance;

	vec3 origin;
	vec3 direction;
	vec3 attenuation;

	bool done;
};

layout(push_constant) uniform PC
{
	int depth;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int irragridCount;
	int quadlightCount;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAs;
layout(set = 4, binding = 0, std430) readonly buffer Pointlights { Pointlight light[]; } pointlights;
layout(set = 5, binding = 0, std430) readonly buffer Spotlights { Spotlight light[]; } spotlights;
layout(set = 5, binding = 1) uniform sampler2DShadow spotlightShadowmap[];
layout(set = 6, binding = 0, std430) readonly buffer Dirlights { Dirlight light[]; } dirlights;
layout(set = 6, binding = 1) uniform sampler2DShadow dirlightShadowmap[];
layout(set = 7, binding = 0, std430) readonly buffer Irragrids { Irragrid grid[]; } irragrids;
layout(set = 7, binding = 1) uniform samplerCubeArray irradianceSamplers[];
layout(set = 8, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {
	Surface surface = surfaceFromGeometryGroup();

	vec3 radiance = vec3(0);

	// DIRECT
	{
		// for each light
//		for(int i = 0; i < pointlightCount; ++i) { // WIP:
//			Pointlight pl = pointlights.light[i];
//			radiance +=  Pointlight_EstimateDirect(topLevelAs, pl, surface);
//		}

		for(int i = 0; i < spotlightCount; ++i) {
			Spotlight sl = spotlights.light[i];
			radiance += Spotlight_EstimateDirect(sl, spotlightShadowmap[nonuniformEXT(i)], surface);
		}

		for(int i = 0; i < dirlightCount; ++i) {
			Dirlight dl = dirlights.light[i];
			radiance += Dirlight_EstimateDirect(dl, dirlightShadowmap[nonuniformEXT(i)], surface);
		}

//		for(int i = 0; i < quadlightCount; ++i) { WIP: handle using the direct tricks and point light shadows
//			Quadlight ql = quadlights.light[i];
//			radiance += Quadlight_FastContribution(topLevelAs, ql, surface);
//		}

			// INDIRECT Diffuse WIP:
		//    {
		//		for(int i = 0; i < irragridCount; ++i) {
		//			Irragrid ig = irragrids.grid[i];
		//			radiance += Irragrid_Contribution(ig, irradianceSamplers[nonuniformEXT(i)], surface);
		//		}
		//    }

			// if roughness...
			// INDIRECT Specular -> reflprobe
			//if(surface.a >= SPEC_THRESHOLD) {
			//	prd.radiance = radiance + surface.emissive;
			//	return;
			//}
	}

	prd.radiance = radiance + surface.emissive;

	// Next ray

	if(surface.a >= SPEC_THRESHOLD) {
		prd.done = true;
		return;
	}

	prd.attenuation = vec3(sampleSpecularBRDF(surface));
	prd.attenuation *= absNdot(surface.o);
	prd.origin = surface.position;
	prd.direction = getOutgoingDir(surface);	
}
