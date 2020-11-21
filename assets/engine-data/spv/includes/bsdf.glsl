#ifndef bsdf_glsl
#define bsdf_glsl

#include "fresnel.glsl"

// SMATH:
float D_GGX(float NoH, float _a) {
    float a = NoH * _a;
    float k = a / (1.0 - NoH * NoH + a * a);
    return k * k * INV_PI;
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL); // full is 2.0 * NoL * NoV / (GGXV + GGXL); however we simplify with the brdf denom later
}

float V_SmithGGXCorrelatedFast(float NoV, float NoL, float a) {
    float GGXV = NoL * (NoV * (1.0 - a) + a);
    float GGXL = NoV * (NoL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL); // full is 2.0 * NoL * NoV / (GGXV + GGXL); however we simplify with the brdf denom later
}

float G_SchlicksmithGGX(float NoL, float NoV, float a)
{
	float k = (a * a) / 2.0;
	float GL = NoL / (NoL * (1.0 - k) + k);
	float GV = NoV / (NoV * (1.0 - k) + k);
	return GL * GV;
}

// Converts a square of roughness to a Phong specular power
float RoughnessSquareToSpecPower(float a) 
{
    return max(0.01, 2.0 / (a + 1e-4) - 2.0);
}

float PhongSpecular(float NoH, float a)
{
    return pow(NoH, RoughnessSquareToSpecPower(a));
}

vec3 CookTorranceGGXSmithSpecular(float NoV, float NoL, float NoH, float a)
{
    float D = D_GGX(NoH, a);
    float G = G_SchlicksmithGGX(NoL, NoV, a);

    float denom = 4 * NoL * NoV;
    return vec3(D * G / denom);
}

vec3 CookTorranceGGXSmithCorrelatedSpecular(float NoV, float NoL, float NoH, float a)
{
    float D = D_GGX(NoH, a);
    float V = V_SmithGGXCorrelated(NoV, NoL, a);

    return vec3(D * V); // denominator simplified with G (= V)
}

vec3 DisneyDiffuse(float NoL, float NoV, float LoH, float a, vec3 albedo) 
{
    float f90 = 0.5 + LoH * LoH  * a;

    return albedo * INV_PI * F_Schlick(NoL, f90) * F_Schlick(NoV, f90);
}

vec3 LambertianDiffuse(vec3 albedo) 
{
    return albedo * INV_PI;
}

#endif