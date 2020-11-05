#ifndef mirror_glsl
#define mirror_glsl

struct Pointlight {
		vec3 position;
		float pad0;

		vec3 color;
		float pad3;

		float intensity;

		float constantTerm;
		float linearTerm;
		float quadraticTerm;

		float radius;

		int samples;
		int hasShadow;
};

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

struct samplerRef {
	int index;
};

struct GltfMat {
	// factors
	vec4 baseColorFactor;
	vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// alpha mask
	float alphaCutoff;
	int mask;

	samplerRef baseColor;
	samplerRef metallicRough;
	samplerRef occlusion;
	samplerRef normal;
	samplerRef emissive;
};

struct Reflprobe {
	int lodCount;
};

struct hitPayload
{
	vec3 radiance;
	int depth;
};

layout(push_constant) uniform PC
{
		int mirrorDepth;
		int pointlightCount;
        int reflprobeCount;
};


#endif