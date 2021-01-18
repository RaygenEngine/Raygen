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
    if(surface.a < SPEC_THRESHOLD){
        pdf = 1.0; // the pdf isn't one here, it is just canceled out with the d of the specular bsdf
        return sampleSpecularBRDF(surface);
    }

    pdf = importanceReflectionSampleGGX(surface, seed);
    return microfacetBRDF(surface); 
}

float sampleBTDF(inout Surface surface, out float pdf, inout uint seed)
{
    if(surface.a < SPEC_THRESHOLD){
        pdf = 1.0; // the pdf isn't one here, it is just canceld out with the d of the specular bsdf
        return sampleSpecularBTDF(surface);
    }

    pdf = importanceRefractionSampleGGX(surface, seed);
    return microfacetBTDF(surface); 
}

vec3 sampleNonSpecularBRDF(inout Surface surface, bool isDiffusePath, out float pdf, inout uint seed)
{
    pdf = nonSpecularReflectionSample(surface, isDiffusePath, seed);
	return nonSpecularBRDF(surface, isDiffusePath);
}

// we split bsdf pdf from other pdfs for MIS use
bool sampleBSDF(inout Surface surface, out vec3 bsdf, out float pdf_path, out float pdf_bsdf, out bool isDiffusePath, out bool isRefractedPath, inout uint seed) {
	// mirror H = N 
	float LoH = absNdot(surface.i);
	surface.h = vec3(0, 0, 1); // surface space N

	float eta = surface.eta_i / surface.eta_o;

	if(surface.a >= SPEC_THRESHOLD) {
        sampleMicrosurfaceNormal(surface, seed);
		LoH = absdot(surface.i, surface.h);
	}

    float k = 1.0 - eta * eta * (1.0 - LoH * LoH);

	isRefractedPath = false;
	isDiffusePath = false;
    bsdf = vec3(1);
    pdf_path = 1.f;

	vec3 kr = interfaceFresnel(surface);

	// handle total intereflection
	float p_reflect = k < 0.0 ? 1.0 : max(kr);

	// transmission
	if(rand(seed) > p_reflect) {
		
		bsdf *= 1.0 - kr; // kt
		pdf_path *= 1 - p_reflect;
		
		float p_transparency = 1.0 - surface.opacity; // TODO: trans material

		// diffuse 
		if(rand(seed) > p_transparency) {
        	bsdf *= sampleDiffuseBRDF(surface, pdf_bsdf, seed);
			pdf_path *= 1 - p_transparency;
			isDiffusePath = true;
		}

		// refraction
		else {
			pdf_path *= p_transparency;
			isRefractedPath = true;

			// specular
			if(surface.a < SPEC_THRESHOLD) {
				bsdf *= sampleSpecularBTDF(surface);
				pdf_bsdf = 1.f; // the pdf isn't one here, it is just canceld out with the d of the specular bsdf
			}

			// glossy
			else {
				surface.o = refract(-surface.i, surface.h, eta);		
				bsdf *= microfacetBTDF(surface);
				pdf_bsdf = importanceRefractionSamplePdf(surface);
			}
		}
	}

	// reflection
	else {
	    bsdf *= kr;
		pdf_path *= p_reflect;

		// specular
		if(surface.a < SPEC_THRESHOLD){
			bsdf *= sampleSpecularBRDF(surface);
			pdf_bsdf = 1.f; // the pdf isn't one here, it is just canceld out with the d of the specular bsdf
		}

		// glossy
		else {
			surface.o = reflect(-surface.i, surface.h);	

			bsdf *= microfacetBRDF(surface);
			pdf_bsdf = importanceReflectionSamplePdf(surface);
		}
	}

	// BIAS: stop erroneous paths
	return !(isRefractedPath && !isOutgoingDirPassingThrough(surface) || // reflect but under actual geometry
	         !isRefractedPath && isOutgoingDirPassingThrough(surface) || // transmit but above actual geometry        
	         pdf_path * pdf_bsdf < BIAS                            // very small pdf
			 );                          
}

//bool sampleSpecularBSDF(inout Surface surface, out vec3 bsdf) {
//
//	float LoH = absNdot(surface.i);
//	surface.h = vec3(0, 0, 1); // surface space N
//
//	bool isRefractedPath = false;
//    float pdf_path = 1.f;
//
//	float eta = surface.eta_i / surface.eta_o;
//
//    float k = 1.0 - eta * eta * (1.0 - LoH * LoH);
//
//	vec3 kr = interfaceFresnel(surface);
//
//	// handle total intereflection
//	float p_reflect = k < 0.0 ? 1.0 : max(kr);
//
//	// transmission
//	if(rand(seed) > p_reflect) {
//		
//		bsdf *= 1.0 - kr; // kt
//		pdf_path *= 1 - p_reflect;
//		
//		float p_transparency = 1.0 - surface.opacity; // TODO: trans material
//
//		// absorb 
//		if(rand(seed) > p_transparency) {
//			bsdf *= 0;
//		}
//		// refraction
//		else {
//			pdf_path *= p_transparency;
//			isRefractedPath = true;
//
//			bsdf *= sampleSpecularBTDF(surface);
//		}
//	}
//
//	// reflection
//	else {
//	    bsdf *= kr;
//		pdf_path *= p_reflect;
//
//		bsdf *= sampleSpecularBRDF(surface);
//	}
//
//	bsdf /= pdf_path;
//
//	// BIAS: stop erroneous paths
//	return !(isRefractedPath && !isOutgoingDirPassingThrough(surface) || // reflect but under actual geometry
//	         !isRefractedPath && isOutgoingDirPassingThrough(surface) || // transmit but above actual geometry        
//	         pdf_path < BIAS                            // very small pdf
//			 ); 
//}

// PERF: only for npt probably, avoid a lot of operations by simplifying terms
//bool sampleBSDF_termElimination...

#endif
