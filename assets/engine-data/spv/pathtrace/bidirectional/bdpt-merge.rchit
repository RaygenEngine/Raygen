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

struct mergePayload
{
	vec3 target;
	vec3 wi;

	vec3 connectionFactor;

	int visible;
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

layout(location = 1) rayPayloadInEXT mergePayload prd;

void main() {
	
	// PERF:
	vec3 tempPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	if(distance(prd.target, tempPos) > BIAS) {
		prd.visible = 0;
		return;
	}

	Surface surface = surfaceFromGeometryGroup();

	prd.connectionFactor = explicitBRDFcosTheta(surface, prd.wi);
	prd.visible = 1;
}
