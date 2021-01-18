#ifndef surface_shading_glsl
#define surface_shading_glsl

#include "shading-math.glsl"

// it helps to decouple this from the bsdf calculations and use it later as kd or kr terms
vec3 interfaceFresnel(Surface surface) 
{
    return F_Schlick(absdot(surface.i, surface.h), surface.f0);
}

vec3 diffuseBRDF(Surface surface)
{
    return LambertianDiffuse(surface.albedo);
}

float specularBRDF(Surface surface)
{
    return 1.0 / absNdot(surface.o);
}

float microfacetBRDF(Surface surface) 
{
    float D = D_GGX(surface.h, surface.a);
    float G = G_Smith(surface.i, surface.h, surface.o, surface.a);

    float numerator = D * G;
    float denominator = 4.0 * absNdot(surface.i) * absNdot(surface.o);

    return numerator / max(denominator, 0.001); 
}

float specularBTDF(Surface surface)
{
    float eta = surface.eta_o / surface.eta_i;
    return eta * eta / absNdot(surface.o);
}

float microfacetBTDF(Surface surface) 
{
    float D = D_GGX(surface.h, surface.a);
    float G = G_Smith(surface.i, surface.h, surface.o, surface.a);

    float dots = absdot(surface.i, surface.h) * absdot(surface.o, surface.h);
    dots /= absNdot(surface.i) * absNdot(surface.o);

    float numerator = surface.eta_o * surface.eta_o * D * G;
    float denominator = surface.eta_i * dot(surface.i, surface.h) + surface.eta_o * dot(surface.o, surface.h); 
    denominator *= denominator;

    return dots * numerator / denominator; 
}

vec3 nonSpecularBRDF(Surface surface, bool isDiffusePath)
{
	vec3 ks = interfaceFresnel(surface);
    vec3 kt = 1.0 - ks;
    vec3 kd = kt * surface.opacity; // TODO: trans material

	return isDiffusePath ? kd * diffuseBRDF(surface) : ks * microfacetBRDF(surface);
}

vec3 nonSpecularBRDF(Surface surface)
{
	vec3 ks = interfaceFresnel(surface);
    vec3 kt = 1.0 - ks;
    vec3 kd = kt * surface.opacity; // TODO: trans material

	return kd * diffuseBRDF(surface) + ks * microfacetBRDF(surface);
}

float hackSpecularBRDF(Surface surface)
{
    return BlinnPhongSpecular(absNdot(surface.h), surface.a);
}

vec3 explicitBRDFcosTheta(Surface surface, vec3 L)
{
    addOutgoingDir(surface, L);

    vec3 ks = interfaceFresnel(surface);
    vec3 kt = 1.0 - ks;
    vec3 kd = kt * surface.opacity;

    vec3 brdf_d = diffuseBRDF(surface);
 
    float brdf_r = surface.a >= SPEC_THRESHOLD ? microfacetBRDF(surface) : hackSpecularBRDF(surface);

    return (kd * brdf_d + ks * brdf_r) * absNdot(surface.o);
}

// term elimination of fs * |o . n| / pdf
//float finalWeight(Surface surface) 
//{ 
//    // PERF: this maybe wrong
//    return absdot(surface.i, surface.h) * G_Smith(surface.i, surface.h, surface.o, surface.a) / (absNdot(surface.i) * absNdot(surface.h));
//}

#endif
