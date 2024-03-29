#ifndef pt_lights_glsl
#define pt_lights_glsl

#include "random.glsl"
#include "sampling.glsl"
#include "shading-math.glsl"
#include "surface.glsl"

struct ShadowPayload
{
	// if -1 this is a delta light - so you can only check with the miss shader
	// else this is an area light with this id
	int id; 
	// t_hit - usefull for area lights that we don't know their position
	float dist;
	// true - there is visibility to our light
	bool hit; 
};

layout(location = 1) rayPayloadEXT ShadowPayload prdShadow;

bool PtLights_ShadowRayTest(accelerationStructureEXT topLevelAs, int id, vec3 origin, vec3 direction, float tMin, float tMax)
{ 
	prdShadow.hit = false;
	prdShadow.id = id;
	prdShadow.dist = 0.0;

    uint  rayFlags =  gl_RayFlagsCullFrontFacingTrianglesEXT;

	// trace ray
	traceRayEXT(topLevelAs,     // acceleration structure
				rayFlags,       // rayFlags
				0xFF,           // cullMask - none
				2,              // sbtRecordOffset - shadow shaders offset
				0,              // sbtRecordStride
				1,              // shadow missIndex
				origin,         // ray origin
				tMin,           // ray min range
				direction,      // ray direction
				tMax,           // ray max range
				1               // payload (location = 1)
	);

	return prdShadow.hit;
}

vec3 Arealight_SampleLi(accelerationStructureEXT topLevelAs, Quadlight ql, int areaLightId, inout Surface surface, out float pdf_light, inout uint seed)
{
	pdf_light = 0;

	vec2 u = rand2(seed) * 2 - 1;
	u.x *= ql.width / 2.f;
	u.y *= ql.height / 2.f;

	vec3 samplePoint =  ql.center + u.x * ql.right + u.y * ql.up;

	vec3 L = normalize(samplePoint - surface.position);

	float cosTheta_o = dot(ql.normal, -L);

	if (cosTheta_o < ql.cosAperture) {
		return vec3(0.0); // behind light
	}

	cosTheta_o = abs(cosTheta_o);

	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) {
		return vec3(0.0);
	}

	float dist = distance(samplePoint, surface.position);
	if(!PtLights_ShadowRayTest(topLevelAs, areaLightId, surface.position, L, 0.001, INF)) {
		return vec3(0.0); // V
	}

	// pdfw = pdfA / (cosTheta_o / r^2) = r^2 * pdfA / cosTheta_o
	pdf_light = (dist * dist) / (ql.width * ql.height * cosTheta_o);

	return ql.color * ql.intensity;
}

float Arealight_PdfLi(accelerationStructureEXT topLevelAs, Quadlight ql, int areaLightId, Surface surface)
{
	vec3 L = getOutgoingDir(surface);

	float cosTheta_o = dot(ql.normal, -L);

	if (cosTheta_o < ql.cosAperture) {
		return 0; // behind light
	}

	if(!PtLights_ShadowRayTest(topLevelAs, areaLightId, surface.position, L, 0.001, INF)) {
		return 0; // zero, in shadow
	}

	// pdfw = pdfA / (cosTheta_o / r^2) = r^2 * pdfA / cosTheta_o
	return (prdShadow.dist * prdShadow.dist) / (ql.width * ql.height * cosTheta_o);
}

float PowerHeuristic(uint numf, float fPdf, uint numg, float gPdf) 
{
    float f = numf * fPdf;
    float g = numg * gPdf;

    return (f * f) / (f * f + g * g);
}

// MIS
vec3 Arealight_EstimateDirect(accelerationStructureEXT topLevelAs, Quadlight ql, int areaLightId, Surface surface, inout uint seed)
{
	vec3 directLighting = vec3(0);

	float pdf_light;
	float pdf_brdf;
	vec3 Li;
	vec3 f;
	float weight;

	// Sample light

	Li = Arealight_SampleLi(topLevelAs, ql, areaLightId, surface, pdf_light, seed);

	vec3 ks = interfaceFresnel(surface);

	// diffuse component
	directLighting += pdf_light >= BIAS ? Li * absNdot(surface.o) * (1.0 - ks) * surface.opacity * diffuseBRDF(surface) / pdf_light : vec3(0);

	if(pdf_light >= BIAS){
		pdf_brdf = importanceReflectionSamplePdf(surface);
								// if passing thourgh pdf = 0
		if(pdf_brdf >= BIAS && !isOutgoingDirPassingThrough(surface)){
			f = ks * microfacetBRDF(surface);
			weight = PowerHeuristic(1, pdf_light, 1, pdf_brdf);

			directLighting += Li * f * absNdot(surface.o) * weight / pdf_light;
		}
	}

	// Sample bsdf
	
	f = vec3(sampleBRDF(surface, pdf_brdf, seed));
	f *= interfaceFresnel(surface);
	
						   // if passing thourgh pdf = 0
	if(pdf_brdf >= BIAS && !isOutgoingDirPassingThrough(surface)) {
		pdf_light = Arealight_PdfLi(topLevelAs, ql, areaLightId, surface);

		// didn't hit light
		if(pdf_light < BIAS) {
			return directLighting;
		}

		Li = ql.color * ql.intensity;
		weight = PowerHeuristic(1, pdf_brdf, 1, pdf_light);

		directLighting += Li * f * absNdot(surface.o) * weight / pdf_brdf;
	}

	return directLighting;
}

vec3 Pointlight_EstimateDirect(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	vec3 L = normalize(pl.position - surface.position);

	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) {
		return vec3(0.0);
	}

	float dist = distance(pl.position, surface.position);
	if(pl.hasShadow == 1 && !PtLights_ShadowRayTest(topLevelAs, -1, surface.position, L, 0.001, dist)) {
		return vec3(0.0); // V
	}

	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
  			     pl.quadraticTerm * (dist * dist));

	vec3 Li = pl.color * pl.intensity * attenuation;  
	return Li * explicitBRDFcosTheta(surface);
}

vec3 Spotlight_EstimateDirect(accelerationStructureEXT topLevelAs, Spotlight sl, Surface surface)
{
	vec3 L = normalize(sl.position - surface.position);

	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) {
		return vec3(0.0);
	}

	// spot effect (soft edges)
	float theta = dot(L, -sl.front);
    float epsilon = (sl.innerCutOff - sl.outerCutOff);
    float spotEffect = clamp((theta - sl.outerCutOff) / epsilon, 0.0, 1.0);

	if(spotEffect < BIAS){
		return vec3(0.0);
	}

	float dist = distance(sl.position, surface.position);
	if(sl.hasShadow == 1 && !PtLights_ShadowRayTest(topLevelAs, -1, surface.position, L, 0.001, dist)) {
		return vec3(0.0); // V
	}

	float attenuation = 1.0 / (sl.constantTerm + sl.linearTerm * dist + 
  			     sl.quadraticTerm * (dist * dist));

	vec3 Li = sl.color * sl.intensity * attenuation * spotEffect;  
	return Li * explicitBRDFcosTheta(surface);
}

vec3 Dirlight_EstimateDirect(accelerationStructureEXT topLevelAs, Dirlight dl, Surface surface)
{
	vec3 L = normalize(-dl.front);

	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) {
		return vec3(0.0);
	}

	if(dl.hasShadow == 1 && !PtLights_ShadowRayTest(topLevelAs, -1, surface.position, L, 0.001, INF)) {
		return vec3(0.0); // V
	}

	vec3 Li = dl.color * dl.intensity; 
	return Li * explicitBRDFcosTheta(surface);
}

#endif
