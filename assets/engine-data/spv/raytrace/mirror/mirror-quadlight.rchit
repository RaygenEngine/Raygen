#extension GL_EXT_nonuniform_qualifier : enable

#include "global-descset.glsl"

struct hitPayload {
	vec3 radiance;

	vec3 origin;
	vec3 direction;
	vec3 attenuation;

	bool done;
};

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 8, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {

	int quadId = gl_InstanceCustomIndexEXT;
	Quadlight ql = quadlights.light[nonuniformEXT(quadId)];

	prd.done = true;

	float LnoL = dot(ql.normal, -gl_WorldRayDirectionEXT);

		// behind
	if(LnoL < BIAS) {
		prd.radiance = vec3(0);
		return;
	}

	prd.radiance = ql.color * ql.intensity;
}
