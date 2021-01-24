#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

struct ShadowPayload {
	int id;
	float dist;
	bool hit; 
};

layout(location = 0) rayPayloadInEXT ShadowPayload prd;

void main() {
	prd.hit = false; // in shadow
}
