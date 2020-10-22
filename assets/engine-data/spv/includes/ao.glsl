#ifndef ao_glsl
#define ao_glsl

struct hitPayload
{
	float md;
};

layout(push_constant) uniform PC
{
	int samples;
};


#endif