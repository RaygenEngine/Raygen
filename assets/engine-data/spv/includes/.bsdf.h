#ifndef bsdf_h
#define bsdf_h

// TODO: remove this file

#include "random.h"

// DOC:
// a = perceptualRoughness * perceptualRoughness

// CHECK: math
float D_GGX(float NoH, float _a) {
    float a = NoH * _a;
    float k = a / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

// PERF:
float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL + 1e-5);
}

vec3 F_Schlick(float NoV, vec3 f0) {
    float f = pow(1.0 - NoV, 5.0);
    return f + f0 * (1.0 - f);
}

vec3 F_Schlick(float NoV, vec3 f0, vec3 f90) {
    return f0 + (f90 - f0) * pow(1.0 - NoV, 5.0);
}

float F_Schlick(float NoV, float f0, float f90) {
    return f0 + (f90 - f0) * pow(1.0 - NoV, 5.0);
}

vec3 Fd_Burley(float NoV, float NoL, float LoH, vec3 diffuseColor, float a) {
    float f90 = 0.5 + 2.0 * a * LoH * LoH;
    float lightScatter = F_Schlick(NoL, 1.0, f90);
    float viewScatter = F_Schlick(NoV, 1.0, f90);
    return diffuseColor * (lightScatter * viewScatter * (1.0 / PI));
}

vec3 Fr_CookTorranceGGX(float NoV, float NoL, float NoH, float LoH, vec3 f0, float a)
{
    float D = D_GGX(NoH, a);

    // CHECK:
    vec3 f90 = vec3(0.5 + 2.0 * a * LoH * LoH);

    vec3  F = F_Schlick(LoH, f0, f90);
    float V = V_SmithGGXCorrelated(NoV, NoL, a);

    return (D * V) * F; // denominator simplified with G
}

// Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL, float dotNV, float a)
{
	float k = (a * a) / 2.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

vec3 importanceSampleGGX(vec2 Xi, float a, vec3 normal) 
{
    const float phi = 2.0f * PI * Xi.x;
    // (aa-1) == (a-1)(a+1) produces better fp accuracy
    const float cosTheta2 = (1 - Xi.y) / (1 + (a + 1) * ((a - 1) * Xi.y));
    const float cosTheta = sqrt(cosTheta2);
    const float sinTheta = sqrt(1 - cosTheta2);

    vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	// Tangent space
	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangentX = normalize(cross(up, normal));
	vec3 tangentY = normalize(cross(normal, tangentX));

	// Convert to world Space
	return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}
	
#endif