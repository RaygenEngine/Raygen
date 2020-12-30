#ifndef pt_fresnelpath_glsl
#define pt_fresnelpath_glsl

#include "surface.glsl"

void FresnelPath(inout Surface surface, out vec3 attenuation, out float pathPdf, out float bsdfPdf, out bool isRefl, inout uint seed) {
	// mirror H = N 
	float LoH = max(Ndot(surface.v), BIAS);
	vec3 H = vec3(0, 0, 1); // surface space N
	float k = 1.0 - surface.eta * surface.eta * (1.0 - surface.nov * surface.nov);

	if(surface.a >= SPEC_THRESHOLD) {
		vec2 u = rand2(seed);
		H = importanceSampleGGX(u, surface.a);
		LoH = max(dot(surface.v, H), BIAS);
		k = 1.0 - surface.eta * surface.eta * (1.0 - LoH * LoH); // NoH = LoH
	}

	isRefl = true;

	vec3 kr = F_Schlick(LoH, surface.f0);

	// handle total intereflection
	float p_reflect = k < 0.0 ? 1.0 : sum(kr) / 3.f;

	// transmission
	if(rand(seed) > p_reflect) {
		
		attenuation = 1.0 - kr; // kt
		pathPdf = 1 - p_reflect;
		
		// WIP: rename
		float p_transparency = 1.0 - surface.opacity;

		// diffuse 
		if(rand(seed) > p_transparency) {
			attenuation *= SampleDiffuseDirection(surface, seed, bsdfPdf);
			pathPdf *= 1 - p_transparency;
		}

		// refraction
		else {
			pathPdf *= p_transparency;
			isRefl = false;

			// specular
			if(surface.a < SPEC_THRESHOLD) {
				attenuation *= SampleSpecularTransmissionDirection(surface);
				bsdfPdf = 1.f;
			}

			// glossy
			else {
				surface.l = refract(-surface.v, H, surface.eta);		
				cacheSurfaceDots(surface);

				attenuation *= microfacetBtdfNoL(surface);
				bsdfPdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
			}
		}
	}

	// reflection
	else {
	    attenuation = kr;
		pathPdf = p_reflect;

		// mirror
		if(surface.a < SPEC_THRESHOLD){
			attenuation *= SampleSpecularReflectionDirection(surface);
			bsdfPdf = 1.f;
		}

		// glossy
		else {
			surface.l =  reflect(-surface.v, H);		
			cacheSurfaceDots(surface);

			attenuation *= microfacetBrdfNoL(surface);
			bsdfPdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
		}
	}
}

#endif
