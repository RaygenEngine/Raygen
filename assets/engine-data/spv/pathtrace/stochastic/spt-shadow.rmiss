struct ShadowPayload {
	int id;
	float dist;
	bool hit;  
};

layout(location = 1) rayPayloadInEXT ShadowPayload prd;

void main() {
	// if delta light (id == -1) this check is enough
	prd.hit = prd.id != -1 ? false : true;
}
