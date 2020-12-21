#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

struct shadowHitPayload
{
	vec3 filt;
	uint seed;
};

layout(location = 1) rayPayloadInEXT shadowHitPayload prd;

layout(push_constant) uniform PC
{
	int bounces;
	int frame;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int quadlightCount;
};

void main() {
}
