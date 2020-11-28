#ifndef rtspec_glsl
#define rtspec_glsl

struct hitPayload
{
	vec3 radiance;
	int depth;
};

layout(push_constant) uniform PC
{
	int depth;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int irragridCount;
};

#endif
