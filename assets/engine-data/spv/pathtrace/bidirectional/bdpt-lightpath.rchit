#include "pathtrace/gltf-surface.glsl"

struct hitPayload
{
	vec3 origin; 
	vec3 direction;
	vec3 normal;
	vec3 throughput;

	int hitType; 
	uint seed;
};

layout(location = 1) rayPayloadInEXT hitPayload prd;

void main() {
	
	Surface surface = surfaceFromGeometryGroup();

	bool isSpecialPath;
	float pdf;
	if(!sampleBSDF(surface, prd.throughput, pdf, isSpecialPath, prd.seed)) {
		prd.throughput = vec3(0);
		prd.hitType = 1;
		return;
	}

	// projection solid angle form
	pdf /= absNdot(surface.o);

	prd.normal = surface.basis.normal;
	prd.throughput /= pdf;
	prd.hitType = 0; // continue
	prd.origin = surface.position;
	prd.direction = getOutgoingDir(surface);
}
