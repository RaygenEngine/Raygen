#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

layout(location = 1) rayPayloadInEXT bool prd;

void main() {
	prd = false; // in shadow
}
