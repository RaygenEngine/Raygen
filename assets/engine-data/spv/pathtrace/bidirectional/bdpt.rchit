#include "pathtrace/gltf-surface.glsl"
#include "pathtrace/lights.glsl"

struct hitPayload
{
	vec3 origin; 
	vec3 direction;
	vec3 normal;
	vec3 throughput;

	int hitType; 
	uint seed;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
	// TODO:	
}
