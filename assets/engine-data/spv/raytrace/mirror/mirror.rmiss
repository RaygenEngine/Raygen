#include "sky.glsl"

struct hitPayload
{
	vec3 radiance;

	vec3 origin;
	vec3 direction;
	vec3 attenuation;

	bool done;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	prd.radiance = GetSkyColor(gl_WorldRayOriginEXT, gl_WorldRayDirectionEXT);
	prd.done = true;
}
