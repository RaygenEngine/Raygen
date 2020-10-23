#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"
#include "raytrace/ao/ao.glsl"

layout(location = 0) rayPayloadInEXT hitPayload inPrd;

void main() {
	inPrd.md = 1.0;
}



