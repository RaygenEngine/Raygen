struct mergePayload
{
	vec3 target;
	vec3 wi;

	vec3 connectionFactor;

	int visible;
};

layout(location = 1) rayPayloadInEXT mergePayload prd;

void main() {
	// PERF:
	vec3 tempPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	if(distance(prd.target, tempPos) > BIAS) {
		prd.visible = 0;
	}
	else {
		prd.visible = 1;
	}
}
