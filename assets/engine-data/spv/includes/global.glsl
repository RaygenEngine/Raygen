#ifndef global_glsl
#define global_glsl

// TODO: auto include at every shader

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef INV_PI
#define INV_PI 0.31830988618379067154f
#endif

#ifndef INV_2PI
#define INV_2PI 0.15915494309189533577f
#endif

#ifndef INV_4PI
#define INV_4PI 0.07957747154594766788f
#endif

#ifndef PI_OVER2
#define PI_OVER2 1.57079632679489661923f
#endif

#ifndef PI_OVER4
#define PI_OVER4 0.78539816339744830961f
#endif

#ifndef SQRT2
#define SQRT2 1.41421356237309504880f
#endif

#ifndef PHI
#define PHI 1.61803398874989484820459f;
#endif

#ifndef BIAS
#define BIAS 1e-4
#endif

#ifndef SPEC_THRESHOLD
#define SPEC_THRESHOLD 0.001
#endif


float saturate(float v)
{
	return clamp(v, 0.0, 1.0);
}

vec2 saturate(vec2 v)
{
	return clamp(v, vec2(0.0), vec2(1.0));
}

vec3 saturate(vec3 v)
{
	return clamp(v, vec3(0.0), vec3(1.0));
}

vec4 sampleCubemapLH(samplerCube cubemap, vec3 directionRH) 
{
	return texture(cubemap, vec3(directionRH.x, directionRH.y, -directionRH.z));
}

float max(vec3 v) {
	return max(v.x, max(v.y, v.z));
}

float min(vec3 v) {
	return min(v.x, min(v.y, v.z));
}

float sum(vec3 v) {
	return v.x + v.y + v.z;
}

float abssum(vec3 v) {
	return abs(v.x) + abs(v.y) + abs(v.z);
}

float abssum(vec2 v) {
	return abs(v.x) + abs(v.y);
}

float luminance(vec3 rgb) {
	// Relative luminance, most commonly used but other definitions exist
	return dot(rgb, vec3(0.2126f, 0.7152f, 0.0722f));
}

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

struct Camera {
	vec3 position;
	float pad0;

	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
};

struct GltfMaterial {
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
};

struct Dirlight {
	vec3 front;
	float pad0;

	mat4 viewProj;

	vec3 color;
	float pad3;

	float intensity;
	float maxShadowBias;
	int samples;
	float sampleInvSpread; 
	int hasShadow;    
};

struct Spotlight {
		vec3 position;
		float pad0;

		vec3 front;
		float pad1;

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
		int hasShadow;    
};

struct Reflprobe {
	int lodCount;
	float radius;
	float irradianceFactor;
	float pad;
	vec3 position;
	float pad1;
};

struct Irragrid {
	int width;
	int height;
	int depth;
	int builtCount;

	vec3 firstPos;
	float distToAdjacent;
};

struct Quadlight {
	vec3 center;
	float pad0;
	vec3 normal; // WIP: pass quat
	float pad1;
	vec3 right;
	float width;
	vec3 up;
	float height;

	vec3 color;
	float pad2;

	float intensity;
	float constantTerm;
	float linearTerm;
	float quadraticTerm;

	float radius;

	int samples;
	int hasShadow;
};


#endif
