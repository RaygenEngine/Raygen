#ifndef pt_glsl
#define pt_glsl


struct hitPayload
{
	vec3 radiance;
	vec3 accumThroughput;

	int depth;
	uint seed;
};

layout(push_constant) uniform PC
{
	vec4 reflPos;
	int pointlightCount;
	float innerRadius;
	int samples;
	int bounces;
};

#endif