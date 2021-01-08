#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

struct mergePayload
{
	vec3 target;
	vec3 wi;

	vec3 connectionFactor;

	int visible;
};

layout(location = 1) rayPayloadInEXT mergePayload prd;

void main() {
	prd.visible = 0;
}
