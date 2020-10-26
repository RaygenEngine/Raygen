#ifndef pt_glsl
#define pt_glsl

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

struct Pointlight {
		vec3 position;
		float pad0;

		vec3 color;
		float pad3;

		float intensity;

		float constantTerm;
		float linearTerm;
		float quadraticTerm;
};

struct hitPayload
{
	vec3 radiance;
	vec3 accumThroughput;

	int depth;
	uint seed;
};

layout(push_constant) uniform PC
{
// WIP: > 128 !
	vec4 reflPos;
	int pointlightCount;
	float innerRadius;
	int samples;
	int bounces;
};


#endif