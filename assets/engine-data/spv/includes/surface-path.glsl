#ifndef surface_path_glsl
#define surface_path_glsl

#include "bsdfs.glsl"
#include "surface.glsl"

// diffuse brdf
vec3 SampleDiffuseDirection(inout Surface surface, inout uint seed, out float pdf)
{
    vec2 u = rand2(seed); 
    surface.l = cosineSampleHemisphere(u);
    cacheSurfaceDots(surface);

    pdf = cosineHemispherePdf(surface.nol);

    vec3 brdf_d = DisneyDiffuse(surface.nol, surface.nov, surface.loh, surface.a, surface.albedo);
	
	return brdf_d;
}

// returns mirror brdf
float SampleSpecularReflectionDirection(inout Surface surface)
{
    surface.l = reflect(-surface.v);
    cacheSurfaceDots(surface);

    return BlinnPhongSpecular(surface.noh, surface.a); // SMATH:  
}

// returns specular transmission brdf
float SampleSpecularTransmissionDirection(inout Surface surface)
{
    surface.l = refract(-surface.v, surface.eta);
    cacheSurfaceDots(surface);

    return surface.eta * surface.eta / surface.nol; // SMATH: 
}

// returns microfacet brdf
float ImportanceSampleReflectionDirection(inout Surface surface, inout uint seed, out float pdf)
{
    if(surface.a < SPEC_THRESHOLD){
        pdf = 1.0;
        return SampleSpecularReflectionDirection(surface);
    }

    vec2 u = rand2(seed);
    vec3 H = importanceSampleGGX(u, surface.a);

    surface.l =  reflect(-surface.v, H);
    cacheSurfaceDots(surface);

    pdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);

    return microfacetBrdf(surface); 
}

// returns microfacet btdf
float ImportanceSampleTransmissionDirection(inout Surface surface, inout uint seed, out float pdf)
{
    if(surface.a < SPEC_THRESHOLD){
        pdf = 1.0;
        return SampleSpecularTransmissionDirection(surface);
    }

    vec2 u = rand2(seed);
    vec3 H = importanceSampleGGX(u, surface.a);

    surface.l = refract(-surface.v, H, surface.eta);
    cacheSurfaceDots(surface);

    pdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);

    return microfacetBtdf(surface); 
}

// SMATH: check
// split bsdf pdf from other pdfs for MIS use
void SampleBSDF(inout Surface surface, out vec3 bsdf, out float pathPdf, out float bsdfPdf, out bool isRefl, inout uint seed) {
	// mirror H = N 
	float LoH = absNdot(surface.v);
	vec3 H = vec3(0, 0, 1); // surface space N
	float k = 1.0 - surface.eta * surface.eta * (1.0 - surface.nov * surface.nov);

	if(surface.a >= SPEC_THRESHOLD) {
		vec2 u = rand2(seed);
		H = importanceSampleGGX(u, surface.a);
		LoH = absdot(surface.v, H);
		k = 1.0 - surface.eta * surface.eta * (1.0 - LoH * LoH); 
	}

	isRefl = true;

	vec3 kr = F_Schlick(LoH, surface.f0);

	// handle total intereflection
	float p_reflect = k < 0.0 ? 1.0 : sum(kr) / 3.f;

	// transmission
	if(rand(seed) > p_reflect) {
		
		bsdf = 1.0 - kr; // kt
		pathPdf = 1 - p_reflect;
		
		// WIP: rename
		float p_transparency = 1.0 - surface.opacity;

		// diffuse 
		if(rand(seed) > p_transparency) {
			bsdf *= SampleDiffuseDirection(surface, seed, bsdfPdf);
			pathPdf *= 1 - p_transparency;
		}

		// refraction
		else {
			pathPdf *= p_transparency;
			isRefl = false;

			// specular
			if(surface.a < SPEC_THRESHOLD) {
				bsdf *= SampleSpecularTransmissionDirection(surface);
				bsdfPdf = 1.f;
			}

			// glossy
			else {
				surface.l = refract(-surface.v, H, surface.eta);		
				cacheSurfaceDots(surface);

				bsdf *= microfacetBtdf(surface);
				bsdfPdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
			}
		}
	}

	// reflection
	else {
	    bsdf = kr;
		pathPdf = p_reflect;

		// mirror
		if(surface.a < SPEC_THRESHOLD){
			bsdf *= SampleSpecularReflectionDirection(surface);
			bsdfPdf = 1.f;
		}

		// glossy
		else {
			surface.l = reflect(-surface.v, H);		
			cacheSurfaceDots(surface);

			bsdf *= microfacetBrdf(surface);
			bsdfPdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
		}
	}
}


#endif
