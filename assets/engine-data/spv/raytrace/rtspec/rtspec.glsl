#ifndef rtspec_glsl
#define rtspec_glsl

struct hitPayload
{
	vec3 radiance;
	int depth;
	uint seed;
};

layout(push_constant) uniform PC
{
	int depth;
	int samples;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
};

#endif
