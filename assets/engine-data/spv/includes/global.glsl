#ifndef global_glsl
#define global_glsl

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

// CHECK:
#ifndef INF
#define INF 1e+6
#endif

#ifndef SPEC_THRESHOLD
#define SPEC_THRESHOLD 0.001
#endif

#define ALPHA_MODE_OPAQUE 0
#define ALPHA_MODE_MASK 1
#define ALPHA_MODE_BLEND 2

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

float absdot(vec3 v0, vec3 v1) {
	return abs(dot(v0, v1));
}

bool isVectorNan(vec3 v) {
	return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

// a >= 0.0 ? 1.0 : -1.0;
float csign(float a) {	
	return sign(a) + (1.0 - abs(sign(a))); 
}

// a > 0.0 ? 1.0 : -1.0;
float x_p(float a) {
	return sign(sign(a) - 0.5); 
}

vec3 reconstructWorldPosition(float depth, vec2 uv, mat4 viewProjInv)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 worldPos = viewProjInv * clipPos;

	return worldPos.xyz / worldPos.w; // return world space pos xyz
}

vec3 reconstructEyePosition(float depth, vec2 uv, mat4 projInv)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 eyePos = projInv * clipPos;

	return eyePos.xyz / eyePos.w; // return eye space pos xyz
}

#include "global-bsdfspace.glsl"
#include "global-structs.glsl"


#endif
