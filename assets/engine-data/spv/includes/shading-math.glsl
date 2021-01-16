#ifndef shading_math_glsl
#define shading_math_glsl

// math are in local space

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

float D_GGX(vec3 H, float a) {
    float a2        = a * a;
    float NoH       = Ndot(H);
    float cos4Theta = NoH * NoH * NoH * NoH;
    float tan2Theta = Tan2Theta(H);
	
    float nom    = a2 * x_p(NoH);
    float denom  = (a2 + tan2Theta);
    denom        = PI * cos4Theta * denom * denom;
	
    return nom / denom;
}

float G1_GGX(vec3 V, vec3 H, float a) {
    float a2  = a * a;

    float VoH = dot(V, H);
    float NoV = Ndot(V);

    float tan2Theta = Tan2Theta(V);

    float nom = 2 * x_p(VoH / NoV);
    float denom = 1 + sqrt(1 + a2 * tan2Theta);

    return nom / denom;
}
  
float G_Smith(vec3 I, vec3 H, vec3 O, float a) 
{
    return G1_GGX(I, H, a) * G1_GGX(O, H, a);
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
