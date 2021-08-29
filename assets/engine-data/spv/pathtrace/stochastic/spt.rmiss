#include "sky.glsl"

struct hitPayload
{
	vec3 radiance; // previous radiance

	vec3 origin; // stuff of THIS ray
	vec3 direction;
	vec3 attenuation;

	int hitType; // previous hit type
	uint seed;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	prd.radiance = GetSkyColor(prd.origin, prd.direction);
	prd.hitType = 3; 
}
