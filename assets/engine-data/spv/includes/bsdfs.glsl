#ifndef bsdfs_glsl
#define bsdfs_glsl

vec3 F_Schlick(float cosTheta, vec3 f0)
{
  return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickRoughness(float cosTheta, vec3 f0, float a)
{
    return f0 + (max(vec3(1.0 - a), f0) - f0) * pow(1.0 - cosTheta, 5.0);
}   

vec3 F_Epic(float cosTheta, vec3 f0)
{
  return f0 + (1.0 - f0) * pow(2.0, ((-5.55473 * cosTheta) - 6.98316) * cosTheta);
}

float F_Schlick(float cosTheta, float f90) 
{
    return 1.0 + (f90 - 1) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_Schlick(float cosTheta, vec3 f0, vec3 f90)
{
    return f0 + (f90 - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickRoss(float cosTheta, vec3 f0, float a)
{
    // Shlick's approximation for Ross BRDF -- makes Fresnel converge to less than 1.0 when N.V is low
    return f0 + (1 - f0) * pow(1 - cosTheta, 5 * exp(-2.69 * a)) / (1.0 + 22.7 * pow(a, 1.5));
}

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
