#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.glsl"

struct ShadowPayload {
	int id;
	float dist;
	bool hit;  
};

layout(location = 1) rayPayloadInEXT ShadowPayload prd;

void main() {
	// if delta light (id == -1) this check is enough
	prd.hit = prd.id != -1 ? false : true;
}
