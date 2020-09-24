#ifndef rt_global_glsl
#define rt_global_glsl
#include "onb.glsl"
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


struct Spotlight
{
	vec3 position;
	float pad0;
	vec3 direction;
	float pad1;

	// CHECK: could pass this mat from push constants (is it better tho?)
	// Lightmap
	mat4 viewProj;
	vec3 color;
	float pad3;

	float intensity;

	float near;
	float far;

	float outerCutOff;
	float innerCutOff;

	float constantTerm;
	float linearTerm;
	float quadraticTerm;

	float maxShadowBias;
	int samples;
	float sampleInvSpread;
};

layout(push_constant) uniform PC
{
    int frame;
    int depth;
    int samples;
    int convergeUntilFrame;
	int spotlightCount;
};

struct hitPayload
{
	vec3 radiance;
	vec3 accumThroughput;

	int depth;
	uint seed;
};



#endif