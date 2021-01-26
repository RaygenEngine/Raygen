#ifndef surface_bsdf_glsl
#define surface_bsdf_glsl

#include "surface-sampling.glsl"

vec3 sampleDiffuseBRDF(inout Surface surface, out float pdf, inout uint seed)
{
    pdf = cosineSampleHemisphere(surface, seed);
	return diffuseBRDF(surface);
}

float sampleSpecularBRDF(inout Surface surface)
{
    analyticReflectionSample(surface);
    return specularBRDF(surface);
}

float sampleSpecularBTDF(inout Surface surface)
{
    analyticRefractionSample(surface);
    return specularBTDF(surface);
}

float sampleBRDF(inout Surface surface, out float pdf, inout uint seed)
{
    pdf = importanceReflectionSampleGGX(surface, seed);
    return microfacetBRDF(surface); 
}

float sampleBTDF(inout Surface surface, out float pdf, inout uint seed)
{
    pdf = importanceRefractionSampleGGX(surface, seed);
    return microfacetBTDF(surface); 
}

// currently special paths are the refracted and the specular
bool sampleBSDF(inout Surface surface, out vec3 bsdf, out float pdf, out bool isSpecialPath, inout uint seed) {
	
	// sample microfacet direction - or specular
	// mirror H = N 
	float LoH = absNdot(surface.i);
	surface.h = vec3(0, 0, 1); // surface space N

	float eta = surface.eta_i / surface.eta_o;

	if(surface.a >= SPEC_THRESHOLD) {
        sampleMicrosurfaceNormal(surface, seed);
		LoH = absdot(surface.i, surface.h);
	}

    float k = 1.0 - eta * eta * (1.0 - LoH * LoH);
	
	bool isRefractedPath = false;
	isSpecialPath = surface.a < SPEC_THRESHOLD;
    bsdf = vec3(1);
    pdf = 1.0;

	vec3 kr = interfaceFresnel(surface);

	// handle total intereflection
	float p_reflect = k < 0.0 ? 1.0 : max(kr); // max for case of metal surface is better

	// transmission - light that originates from inside surface
	if(rand(seed) > p_reflect) {	
		pdf *= 1 - p_reflect;
		
		float p_transparency = 1.0 - surface.opacity; // TODO: trans material

		// diffuse - light entered and diffusely scattered from the same spot on surface
		if(rand(seed) > p_transparency) {
			float pdfd;
        	bsdf *= sampleDiffuseBRDF(surface, pdfd, seed);
			vec3 kd = 1.0 - interfaceFresnel(surface); // we change
			bsdf *= kd;
			pdf *= pdfd;
			pdf *= (1 - p_transparency);
		}

		// refraction
		else {
			bsdf *= 1.0 - kr; // kt
			pdf *= p_transparency;
			isSpecialPath = true;
			isRefractedPath = true;

			// specular
			if(surface.a < SPEC_THRESHOLD) {
				bsdf *= sampleSpecularBTDF(surface);
			}

			// glossy
			else {
				surface.o = refract(-surface.i, surface.h, eta);		
				bsdf *= microfacetBTDF(surface);
				pdf *= importanceRefractionSamplePdf(surface);
			}
		}
	}

	// reflection light that originates from outside the surface 
	else {
	    bsdf *= kr;
		pdf *= p_reflect;

		// specular
		if(surface.a < SPEC_THRESHOLD){
			bsdf *= sampleSpecularBRDF(surface);
		}

		// glossy
		else {
			surface.o = reflect(-surface.i, surface.h);	

			bsdf *= microfacetBRDF(surface);
			pdf *= importanceReflectionSamplePdf(surface);
		}
	}

	// BIAS: stop erroneous paths
	return !(isRefractedPath && !isOutgoingDirPassingThrough(surface) || // refract but does NOT pass through geometry
	         !isRefractedPath && isOutgoingDirPassingThrough(surface) || // reflect but passing throught geometry        
	         pdf < BIAS                            // very small pdf
			 );                          
}

#endif
