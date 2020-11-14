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
	float innerRadius;
	int samples;
	int bounces;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
};

#endif
