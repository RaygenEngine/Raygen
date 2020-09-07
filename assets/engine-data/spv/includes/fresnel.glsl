#ifndef fresnel_glsl
#define fresnel_glsl

vec3 F_Schlick(float cosTheta, vec3 f0)
{
  return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
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

#endif