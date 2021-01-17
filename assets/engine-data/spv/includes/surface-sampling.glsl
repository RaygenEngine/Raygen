#ifndef surface_sampling_glsl
#define surface_sampling_glsl

#include "random.glsl"
#include "sampling.glsl"
#include "surface-shading.glsl"

void analyticReflectionSample(inout Surface surface)
{
    surface.o = reflect(-surface.i);
    surface.h = vec3(0, 0, 1);
}

void analyticRefractionSample(inout Surface surface)
{
    surface.o = refract(-surface.i, surface.eta_i / surface.eta_o);
    surface.h = vec3(0, 0, 1);
}

float importanceReflectionSamplePdf(Surface surface)
{
    return D_GGX(surface.h, surface.a) * absNdot(surface.h) / (4.0 * absdot(surface.o, surface.h));
}

void sampleMicrosurfaceNormal(inout Surface surface, inout uint seed)
{
    vec2 u = rand2(seed); 

    float theta = atan((surface.a * sqrt(u.x)) / sqrt(1 - u.x));
    float phi = 2.0f * PI * u.y;
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    surface.h = normalize(vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
}

float importanceReflectionSampleGGX(inout Surface surface, inout uint seed)
{
    sampleMicrosurfaceNormal(surface, seed);
    surface.o = reflect(-surface.i, surface.h);

    return importanceReflectionSamplePdf(surface);
}

float importanceRefractionSamplePdf(Surface surface)
{
    float pdf = D_GGX(surface.h, surface.a) * absNdot(surface.h);

    // Jacobian for refraction
    float numerator = surface.eta_o * surface.eta_o * absdot(surface.o, surface.h);
    float denominator = surface.eta_i * dot(surface.i, surface.h) + surface.eta_o * dot(surface.o, surface.h); 
    denominator *= denominator;

    return pdf * numerator / denominator;
}

float importanceRefractionSampleGGX(inout Surface surface, inout uint seed)
{
    sampleMicrosurfaceNormal(surface, seed);
    surface.o = refract(-surface.i, surface.h, surface.eta_i / surface.eta_o);

    return importanceRefractionSamplePdf(surface);
}

float cosineHemisphereSamplePdf(Surface surface)
{
    return cosineHemispherePdf(absNdot(surface.o));
}

float cosineSampleHemisphere(inout Surface surface, inout uint seed)   
{
    surface.o = cosineSampleHemisphere(rand2(seed));
    surface.h = normalize(surface.i + surface.o);

    return cosineHemisphereSamplePdf(surface);
}

float nonSpecularReflectionPdf(Surface surface, bool isDiffusePath)
{
    return isDiffusePath ? cosineHemisphereSamplePdf(surface) : importanceReflectionSamplePdf(surface);
}

float nonSpecularReflectionSample(inout Surface surface, bool isDiffusePath, inout uint seed)
{
	return isDiffusePath ? cosineSampleHemisphere(surface, seed) : importanceReflectionSampleGGX(surface, seed);
}

#endif
