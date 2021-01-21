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

struct hitPayload
{
	vec3 origin; 
	vec3 direction;
	vec3 normal;
	vec3 throughput;

	int hitType; 
	uint seed;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	// TODO:	
}
