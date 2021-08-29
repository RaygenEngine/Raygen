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
