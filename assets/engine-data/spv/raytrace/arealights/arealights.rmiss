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
	int frame;
	int pointlightCount;
	int quadlightCount;
};

layout(location = 0) rayPayloadInEXT hitPayload inPrd;

void main() 
{
	// reached light source
	inPrd.shadow = 0;
}
