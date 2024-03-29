struct ShadowPayload {
	int id;
	float dist;
	bool hit; 
};

layout(location = 0) rayPayloadInEXT ShadowPayload prd;

void main() 
{
	// if we hit the light we searched for
	prd.hit = prd.id != gl_InstanceCustomIndexEXT ? false : true;
	prd.dist = gl_HitTEXT;
}
