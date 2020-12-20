#ifndef bsdf_glsl
#define bsdf_glsl

#include "fresnel.glsl"

float D_GGX(float NoH, float a) {

    float a2     = a * a;
    float NoH2   = NoH * NoH;
	
    float nom    = a2;
    float denom  = (NoH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float G_SchlickGGX(float NoV, float k)
{
    float nom   = NoV;
    float denom = NoV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float G_SmithSchlickGGX(float NoV, float NoL, float k)
{
    float ggx1 = G_SchlickGGX(NoV, k);
    float ggx2 = G_SchlickGGX(NoL, k);
	
    return ggx1 * ggx2;
}

// Converts a square of roughness to a Phong specular power
float RoughnessSquareToSpecPower(float a) 
{
    return max(0.01, 2.0 / (a * a + 1e-4) - 2.0);
}

float BlinnPhongSpecular(float NoH, float a)
{
    return pow(NoH, RoughnessSquareToSpecPower(a));
}

// R is reflect(-L)
float PhongSpecular(float RoV, float a)
{
    return pow(RoV, RoughnessSquareToSpecPower(a));
}

// view dependent
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
