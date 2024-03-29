struct hitPayload {
	vec3 radiance; // previous radiance

	vec3 origin; // stuff of THIS ray
	vec3 direction;
	vec3 attenuation;

	int hitType; // previous hit type
	uint seed;
};

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 7, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() 
{
	int quadId = gl_InstanceCustomIndexEXT;
	Quadlight ql = quadlights.light[quadId];

	float LnoL = dot(ql.normal, -gl_WorldRayDirectionEXT);

	prd.radiance = vec3(0);

	// behind
	if(LnoL < BIAS) {
		prd.hitType = 3;
		return;	
	}

	// direct hit
	if(prd.hitType == 0) {
		prd.radiance = ql.color;
	}

	// from paths delta or refracted path
	if(prd.hitType == 2) {
		prd.radiance = ql.color * ql.intensity;
	}

	prd.hitType = 3;
}
