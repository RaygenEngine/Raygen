#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"
#include "mirror.glsl"

#include "sky.glsl"

layout(location = 0) rayPayloadInEXT hitPayload inPrd;

void main() {
	inPrd.radiance = GetSkyColor(gl_WorldRayOriginEXT, gl_WorldRayDirectionEXT);
}



