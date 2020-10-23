#ifndef bsdf_glsl
#define bsdf_glsl

#include "global-shading.glsl"
#include "fresnel.glsl"

// math here are in bsdf space 
// i.e. change basis of vectors using the normal of the surface

// n is the normal in bsdf space and is equal to vec3(0,0,1)
// w is the omega direction | must be normalized, otherwise math may break

// theta = angle between w and n (z-axis)
// sperical coordinates (theta, phi)

// cosTheta = n o w = (0,0,1) o w = w.z
float Ndot(vec3 w) { return w.z; }
float CosTheta(vec3 w) { return w.z; }
float Cos2Theta(vec3 w) { return w.z * w.z; }
float AbsCosTheta(vec3 w) { return abs(w.z); }

// sin2Theta = 1 - cos2Theta
float Sin2Theta(vec3 w) { return max(0.f, 1.f - w.z * w.z); }
float SinTheta(vec3 w) { return sqrt(Sin2Theta(w)); }

// tanTheta = sinTheta / cosTheta
float TanTheta(vec3 w) { return SinTheta(w) / CosTheta(w); }
float Tan2Theta(vec3 w) { return Sin2Theta(w) / Cos2Theta(w); }
float AbsTanTheta(vec3 w) { return abs(SinTheta(w) / CosTheta(w)); }

// cosPhi = x / sinTheta
float CosPhi(vec3 w) 
{ 
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1, 1);
}

float Cos2Phi(vec3 w) { return CosPhi(w) * CosPhi(w); }

// sinPhi = y / sinTheta
float SinPhi(vec3 w) 
{ 
    float sinTheta = SinTheta(w);
    return sinTheta == 0 ? 1 : clamp(w.y / sinTheta, -1, 1);
}

float Sin2Phi(vec3 w) { return SinPhi(w) * SinPhi(w); }

// DPhi can be found by zeroing the z coordinate of the two vectors to get 2D vectors 
// and then normalizing them. The dot product of these two vectors gives the cosine of the angle between them.
 
float CosDPhi(vec3 wa, vec3 wb) 
{
    return clamp((wa.x * wb.x + wa.y * wb.y) / 
            sqrt((wa.x * wa.x + wa.y * wa.y) *      
                 (wb.x * wb.x + wb.y * wb.y)), -1, 1);
}

bool sameHemisphere(vec3 w, vec3 wp) 
{
    return w.z * wp.z > 0;
}

// reflection on normal
// refl (wo, n) = -wo + 2 * dot(wo, n) * n =
//  = vec3(-wo.x, -wo.y, wo.z); note: opposite signs for consistency with glsl reflect
vec3 reflect(vec3 wo) 
{
    return vec3(wo.x, wo.y, -wo.z);
}

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

vec3 SpecularTerm(float NoV, float NoL, float NoH, float a, vec3 ks)
{
    // CHECK: ks = F should probably be inside the brdfs and omitted here
    if(a < SPEC_THRESHOLD){   
		return ks *  PhongSpecular(NoH, a);
    }

    return ks * CookTorranceGGXSmithCorrelatedSpecular(NoV, NoL, NoH, a);
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

vec3 DiffuseTerm(float NoL, float NoV, float LoH, float a, vec3 albedo, vec3 kd)
{
    return kd * DisneyDiffuse(NoL, NoV, LoH, a, albedo);
}

// same Li and L for both brdfs
vec3 DirectLightBRDF(float NoL, float NoV, float NoH, float LoH, float a, vec3 albedo, vec3 f0)
{
    vec3 ks = F_Schlick(LoH, f0);
    vec3 kd = 1 - ks;
     
    return DiffuseTerm(NoL, NoV, LoH, a, albedo, kd) + SpecularTerm(NoV, NoL, NoH, a, ks);
}

// SMATH: where is D?
vec3 importanceSampleGGX(vec2 u, float a) 
{
    float phi = 2.0f * PI * u.x;
    // (aa-1) == (a-1)(a+1) produces better fp accuracy
    float cosTheta2 = (1 - u.y) / (1 + (a + 1) * ((a - 1) * u.y));
    float cosTheta = sqrt(cosTheta2);
    float sinTheta = sqrt(1 - cosTheta2);

    return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

#endif