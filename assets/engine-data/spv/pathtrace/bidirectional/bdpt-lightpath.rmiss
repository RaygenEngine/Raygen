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
	prd.hitType = 1; // break;
}
