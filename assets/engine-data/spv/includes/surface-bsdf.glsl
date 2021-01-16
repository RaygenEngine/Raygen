#ifndef surface_bsdf_glsl
#define surface_bsdf_glsl

#include "surface-sampling.glsl"

vec3 sampleDiffuseBRDF(inout Surface surface, inout uint seed, out float pdf)
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

float sampleBRDF(inout Surface surface, inout uint seed, out float pdf)
{
    if(surface.a < SPEC_THRESHOLD){
        pdf = 1.0; // the pdf isn't one here, it is just canceled out with the d of the specular bsdf
        return sampleSpecularBRDF(surface);
    }

    pdf = importanceReflectionSampleGGX(surface, seed);
    return microfacetBRDF(surface); 
}

float sampleBTDF(inout Surface surface, inout uint seed, out float pdf)
{
    if(surface.a < SPEC_THRESHOLD){
        pdf = 1.0; // the pdf isn't one here, it is just canceld out with the d of the specular bsdf
        return sampleSpecularBTDF(surface);
    }

    pdf = importanceRefractionSampleGGX(surface, seed);
    return microfacetBTDF(surface); 
}

// we split bsdf pdf from other pdfs for MIS use
bool sampleBSDF(inout Surface surface, out vec3 bsdf, out float pathPdf, out float bsdfPdf, out bool isDiffusePath, inout uint seed) {
	// mirror H = N 
	float LoH = absNdot(surface.i);
	surface.h = vec3(0, 0, 1); // surface space N

	float eta = surface.eta_i / surface.eta_o;

	if(surface.a >= SPEC_THRESHOLD) {
        sampleMicrosurfaceNormal(surface, seed);
		LoH = absdot(surface.i, surface.h);
	}

    float k = 1.0 - eta * eta * (1.0 - LoH * LoH);

	bool isRefl = true;
	isDiffusePath = false;
    bsdf = vec3(1);
    pathPdf = 1.f;

	vec3 kr = interfaceFresnel(surface);

	// handle total intereflection
	float p_reflect = k < 0.0 ? 1.0 : max(kr);

	// transmission
	if(rand(seed) > p_reflect) {
		
		bsdf *= 1.0 - kr; // kt
		pathPdf *= 1 - p_reflect;
		
		float p_transparency = 1.0 - surface.opacity; // TODO: trans material

		// diffuse 
		if(rand(seed) > p_transparency) {
        	bsdf *= sampleDiffuseBRDF(surface, seed, bsdfPdf);
			pathPdf *= 1 - p_transparency;
			isDiffusePath = true;
		}

		// refraction
		else {
			pathPdf *= p_transparency;
			isRefl = false;

			// specular
			if(surface.a < SPEC_THRESHOLD) {
				bsdf *= sampleSpecularBTDF(surface);
				bsdfPdf = 1.f; // the pdf isn't one here, it is just canceld out with the d of the specular bsdf
			}

			// glossy
			else {
				surface.o = refract(-surface.i, surface.h, eta);		
				bsdf *= microfacetBTDF(surface);
				bsdfPdf = importanceRefractionSamplePdf(surface);
			}
		}
	}

	// reflection
	else {
	    bsdf *= kr;
		pathPdf *= p_reflect;

		// specular
		if(surface.a < SPEC_THRESHOLD){
			bsdf *= sampleSpecularBRDF(surface);
			bsdfPdf = 1.f; // the pdf isn't one here, it is just canceld out with the d of the specular bsdf
		}

		// glossy
		else {
			surface.o = reflect(-surface.i, surface.h);	

			bsdf *= microfacetBRDF(surface);
			bsdfPdf = importanceReflectionSamplePdf(surface);
		}
	}

	// BIAS: stop erroneous paths
	return !(isRefl && isOutgoingDirPassingThrough(surface) ||   // reflect but under actual geometry
	         !isRefl && !isOutgoingDirPassingThrough(surface) || // transmit but above actual geometry        
	         pathPdf * bsdfPdf < BIAS                            // very small pdf
			 );                          
}

// PERF: only for npt probably, avoid a lot of operations by simplifying terms
//bool sampleBSDF_termElimination...

#endif
