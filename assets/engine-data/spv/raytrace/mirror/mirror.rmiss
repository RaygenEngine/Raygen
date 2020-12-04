#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

#include "sky.glsl"

struct hitPayload
{
	vec3 radiance;
	int depth;
};

layout(push_constant) uniform PC
{
	int depth;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int irragridCount;
};

layout(location = 0) rayPayloadInEXT hitPayload inPrd;

void main() {
	inPrd.radiance = GetSkyColor(gl_WorldRayOriginEXT, gl_WorldRayDirectionEXT);
}
