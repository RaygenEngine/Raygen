#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

layout(location = 1) rayPayloadInEXT bool prd;

void main() {
	prd = true;
}
