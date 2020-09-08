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

layout(push_constant) uniform Constants
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
	vec3 throughput;

	int depth;
	uint seed;
};


// Fragment Shading Space Info
// Contains space and view data for a specific "fragment" in the world as well as a seed (for utility)
// (Encapsulates all non material specific information about the "fragment")
// This struct should be used for all lighting functions
struct FsSpaceInfo {
	Onb orthoBasis;
	vec3 V; // In shading space
	vec3 worldPos;

	// Optional in some cases (eg: direct lighting) but usefull to have everywhere for simpler interfaces
	uint seed;
};


// All surface information required to perform "standard" brdf.
// (We could have more of these for: 1) btdf 2) different brdf implementations eg: cloth, clear-coating)
struct FragBrdfInfo {
	vec3 diffuseColor;

	vec3 f0; // specularColor (?)
	
	float a; // roughness^2
};

// Does not properly initialize seed
FsSpaceInfo GetFragSpace(vec3 worldNormal, vec3 worldPos, vec3 worldViewDir) {
	FsSpaceInfo fragSpace;
	
	fragSpace.orthoBasis = branchlessOnb(worldNormal);
	fragSpace.worldPos = worldPos;
	fragSpace.V = toOnbSpaceReturn(fragSpace.orthoBasis, normalize(worldViewDir - worldPos));
	fragSpace.seed = 0;
	
	return fragSpace;
}

#endif