#ifndef rt_global_h
#define rt_global_h

// TODO: auto include in rt shaders

struct Vertex
{
	float posX;
	float posY;
	float posZ;
	float nrmX;
	float nrmY;
	float nrmZ;
	float tngX;
	float tngY;
	float tngZ;
	float u;
	float v;
};


struct OldVertex
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 uv;
};

struct hitPayload
{
	vec3 radiance;
	float throughput;

	int depth;
	uint seed;
};

// SMATH:
vec3 offsetRay(vec3 p, vec3 n)
{
	const float intScale   = 256.0f;
	const float floatScale = 1.0f / 65536.0f;
	const float origin     = 1.0f / 32.0f;

	ivec3 of_i = ivec3(intScale * n.x, intScale * n.y, intScale * n.z);

	vec3 p_i = vec3(intBitsToFloat(floatBitsToInt(p.x) + ((p.x < 0) ? -of_i.x : of_i.x)),
					intBitsToFloat(floatBitsToInt(p.y) + ((p.y < 0) ? -of_i.y : of_i.y)),
					intBitsToFloat(floatBitsToInt(p.z) + ((p.z < 0) ? -of_i.z : of_i.z)));

	return vec3(abs(p.x) < origin ? p.x + floatScale * n.x : p_i.x,  
				abs(p.y) < origin ? p.y + floatScale * n.y : p_i.y,  
				abs(p.z) < origin ? p.z + floatScale * n.z : p_i.z);
}

#endif