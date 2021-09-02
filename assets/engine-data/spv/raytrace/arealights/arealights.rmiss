struct ShadowPayload {
	int id;
	float dist;
	bool hit;  
};

layout(location = 0) rayPayloadInEXT ShadowPayload prd;

void main() {
	prd.hit = false; // in shadow
}
