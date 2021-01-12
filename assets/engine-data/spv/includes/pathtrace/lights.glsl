#ifndef pt_lights_glsl
#define pt_lights_glsl

#include "bsdfs.glsl"
#include "random.glsl"
#include "surface.glsl"

layout(location = 1) rayPayloadEXT bool prdShadow;

bool PtLights_ShadowRayTest(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax, uint seed)
{ 
    uint  rayFlags =  gl_RayFlagsCullFrontFacingTrianglesEXT;

	// trace ray
	traceRayEXT(topLevelAs,     // acceleration structure
				rayFlags,       // rayFlags
				0xFD,           // cullMask - quadlights
				2,              // sbtRecordOffset - shadow shaders offset
				0,              // sbtRecordStride
				1,              // shadow missIndex
				origin,         // ray origin
				tMin,           // ray min range
				direction,      // ray direction
				tMax,           // ray max range
				1               // payload (location = 1)
	);

	return prdShadow;
}

vec3 Quadlight_Ldirect(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface, inout uint seed, float pdf_pickLight)
{
	vec2 u = rand2(seed) * 2 - 1;
	u.x *= ql.width / 2.f;
	u.y *= ql.height / 2.f;

	vec3 samplePoint =  ql.center + u.x * ql.right + u.y * ql.up;
	
	vec3 L = normalize(samplePoint - surface.position);

	float LnoL = dot(ql.normal, -L);

	if (LnoL < BIAS) {
		return vec3(0.0); // behind light
	}

	LnoL = abs(LnoL);

	addIncomingLightDirection(surface, L);
	
	if(!isIncidentLightDirAboveSurfaceGeometry(surface)) {
		return vec3(0.0);
	}

	float dist = distance(samplePoint, surface.position);
	if(!PtLights_ShadowRayTest(topLevelAs, surface.position, L, 0.01, dist, seed)) {
		return vec3(0.0);
	}

	// pdfw = pdfA / (cosTheta_o / r^2) = r^2 * pdfA / cosTheta_o
	float pdf_lightArea = (dist * dist) / (ql.width * ql.height * LnoL);
	float pdf_light = pdf_pickLight * pdf_lightArea;
	float pdf_brdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
	float weightedPdf = pdf_light + pdf_brdf;

	vec3 fs = explicitBrdf(surface, L);

	// solid angle form to match the pdfs | dA = (r^2 / cosTheta_o) * dw
	vec3 Li = ql.color * ql.intensity;
	return Li * fs * surface.nol / weightedPdf;
}

#endif
