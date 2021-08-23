#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

struct ShadowPayload {
	int id;
	float dist;
	bool hit; 
};

layout(location = 1) rayPayloadInEXT ShadowPayload prd;

void main() 
{
	// CHECK: more area light types this will need adjustment
	// if we hit the light we searched for
	prd.hit = prd.id != gl_InstanceCustomIndexEXT ? false : true;
	prd.dist = gl_HitTEXT;
}
