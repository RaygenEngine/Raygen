struct hitPayload
{
	vec3 radiance; // to be filled

	vec3 origin; // this ray stuff
	vec3 direction;
	vec3 attenuation; 

	int hitType; 
	uint seed;
};

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 7, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {

	int quadId = gl_InstanceCustomIndexEXT;
	Quadlight ql = quadlights.light[quadId];

	float cosTheta_o = dot(ql.normal, -gl_WorldRayDirectionEXT);

	if (cosTheta_o < ql.cosAperture) {
		prd.radiance = vec3(0); 
		prd.hitType = 2;
		return;
	}

	prd.radiance = ql.color * ql.intensity;  

	// direct hit 
	if(prd.hitType == 0) {
		prd.radiance = ql.color;
	}

	prd.hitType = 2;
}
