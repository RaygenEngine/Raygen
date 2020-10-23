#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"
#include "raytrace/ao/ao.glsl"

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	//vec3 hitPoint = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	prd.md = 0.0;
}










