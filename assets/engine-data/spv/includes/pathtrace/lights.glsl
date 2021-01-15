#ifndef pt_lights_glsl
#define pt_lights_glsl

#include "bsdfs.glsl"
#include "random.glsl"
#include "sampling.glsl"
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

vec3 Quadlight_Ldirect(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface, bool isDiffusePath, float pdf_pickLight, inout uint seed)
{
	vec2 u = rand2(seed) * 2 - 1;
	u.x *= ql.width / 2.f;
	u.y *= ql.height / 2.f;

	vec3 samplePoint =  ql.center + u.x * ql.right + u.y * ql.up;
	
	vec3 L = normalize(samplePoint - surface.position);

	float cosTheta_o = dot(ql.normal, -L);

	if (cosTheta_o < BIAS) {
		return vec3(0.0); // behind light
	}

	cosTheta_o = abs(cosTheta_o);

	addIncomingLightDirection(surface, L);
	
	if(!isIncidentLightDirAboveSurfaceGeometry(surface)) {
		return vec3(0.0);
	}

	float dist = distance(samplePoint, surface.position);
	if(!PtLights_ShadowRayTest(topLevelAs, surface.position, L, 0.001, dist, seed)) {
		return vec3(0.0); // V
	}

	// pdfw = pdfA / (cosTheta_o / r^2) = r^2 * pdfA / cosTheta_o
	float pdf_lightArea = (dist * dist) / (ql.width * ql.height * cosTheta_o);
	float pdf_light = pdf_pickLight * pdf_lightArea;
	float pdf_brdf = isDiffusePath ? cosineHemispherePdf(surface.nol) : importanceSamplePdf(surface.a, surface.noh, surface.loh);
	float weightedPdf = pdf_light + pdf_brdf;

    vec3 ks = F_Schlick(surface.loh, surface.f0);
    vec3 kt = 1.0 - ks;
    vec3 kd = kt * surface.opacity; // WIP:

    vec3 fs = isDiffusePath ? kd * LambertianDiffuse(surface.albedo) : ks * microfacetBrdf(surface);
 
	// solid angle form to match the pdfs | dA = (r^2 / cosTheta_o) * dw
	vec3 Le = ql.color * ql.intensity;
	return Le * fs * surface.nol / weightedPdf;
}

#endif
