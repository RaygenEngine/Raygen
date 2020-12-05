#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
// TODO:
#define RAY
#include "global.glsl"

struct hitPayload
{
	int shadow;
};

layout(push_constant) uniform PC
{
	int pointlightCount;
	int quadlightCount;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() 
{
	// didn't reach source
	prd.shadow = 1;
}
