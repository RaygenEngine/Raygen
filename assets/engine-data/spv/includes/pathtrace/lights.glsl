#ifndef pt_lights_glsl
#define pt_lights_glsl

#include "bsdf.glsl"
#include "random.glsl"
#include "surface.glsl"

struct shadowHitPayload
{
	vec3 filt; 
	uint seed;
};


layout(location = 1) rayPayloadEXT shadowHitPayload prdShadow;

vec3 PtLights_ShadowRayFilter(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax, uint seed)
{ 
	prdShadow.filt = vec3(1.0);
	prdShadow.seed = seed;

    uint  rayFlags =  gl_RayFlagsCullFrontFacingTrianglesEXT;

	// trace ray
	traceRayEXT(topLevelAs,     // acceleration structure
				rayFlags,       // rayFlags
				0xFD,           // cullMask - nothing
				2,              // sbtRecordOffset
				0,              // sbtRecordStride
				1,              // shadow missIndex
				origin,         // ray origin
				tMin,           // ray min range
				direction,      // ray direction
				tMax,           // ray max range
				1               // payload (location = 1)
	);

	return prdShadow.filt;
}

vec3 Quadlight_LightSample(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface, inout uint seed)
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

	float dist = distance(samplePoint, surface.position);
	vec3 sfilter = PtLights_ShadowRayFilter(topLevelAs, surface.position, L, 0.01, dist, seed); 

	float attenuation = (ql.constantTerm + ql.linearTerm * dist + 
	ql.quadraticTerm * (dist * dist));

	// Conversion of the uniform pdf 1/|A| from the area measure (dA) to the solid angle measure (dw)
	float pdf_light = (attenuation) / (ql.width * ql.height * LnoL);
	float pdf_brdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
	float mis_weight = 1.0 / (pdf_light + pdf_brdf);

	vec3 Li = ql.color * ql.intensity * sfilter;
	return Li * SampleWorldDirection(surface, L) * mis_weight;
}

vec3 Pointlight_LightSample(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface, inout uint seed)
{
	vec3 L = normalize(pl.position - surface.position);  

	float dist = distance(pl.position, surface.position);
	vec3 sfilter = PtLights_ShadowRayFilter(topLevelAs, surface.position, L, 0.01, dist, seed); 

	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
	pl.quadraticTerm * (dist * dist));

	vec3 Li = pl.color * pl.intensity * sfilter * attenuation; 
	return Li * SampleWorldDirection(surface, L);
}

vec3 Spotlight_LightSample(accelerationStructureEXT topLevelAs, Spotlight sl, Surface surface, inout uint seed)
{
	vec3 L = normalize(sl.position - surface.position);

	float dist = distance(sl.position, surface.position);
	vec3 sfilter = PtLights_ShadowRayFilter(topLevelAs, surface.position, L, 0.01, dist, seed);

	float attenuation = 1.0 / (sl.constantTerm + sl.linearTerm * dist + 
	sl.quadraticTerm * (dist * dist));

	// spot effect (soft edges)
	float theta = dot(L, -sl.front);
    float epsilon = (sl.innerCutOff - sl.outerCutOff);
    float spotEffect = clamp((theta - sl.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 Li = sl.color * sl.intensity * sfilter * attenuation * spotEffect; 
	return Li * SampleWorldDirection(surface, L);
}

vec3 Dirlight_LightSample(accelerationStructureEXT topLevelAs, Dirlight dl, Surface surface, inout uint seed)
{
	vec3 L = normalize(-dl.front);

	vec3 sfilter = PtLights_ShadowRayFilter(topLevelAs, surface.position, L, 0.01, 10000.f, seed);

	vec3 Li = dl.color * dl.intensity * sfilter; 
	return Li * SampleWorldDirection(surface, L);
}

#endif
