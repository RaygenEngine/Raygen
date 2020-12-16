#ifndef pt_quadlight_glsl
#define pt_quadlight_glsl

#include "bsdf.glsl"
#include "random.glsl"
#include "surface.glsl"

float PtLights_ShadowRayQuery(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
{ 
	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, 
						  topLevelAs, 
						  gl_RayFlagsTerminateOnFirstHitEXT, 
						  0xFD, // except quad lights - WIP: should be lights in total
						  origin, 
						  tMin,
						  direction, 
						  tMax);

	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery)) {
	}
      
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
		// Got an intersection == Shadow
		return 1.0;
	}
	return 0.0;
}

vec3 Quadlight_LightSample(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface, float pdf_select, inout uint seed)
{
	float pdf_area = 1.0 / (ql.width * ql.height);
	float pdf_light = pdf_area * pdf_select;

	vec2 u = rand2(seed) * 2 - 1;
	u.x *= ql.width / 2.f;
	u.y *= ql.height / 2.f;

	vec3 samplePoint =  ql.center + u.x * ql.right + u.y * ql.up;
	
	vec3 L = normalize(samplePoint - surface.position);
	addIncomingLightDirection(surface, L);

	float dist = distance(samplePoint, surface.position);
	if(ql.hasShadow == 1 && PtLights_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, dist) > 0.0){
		return vec3(0.0); // in shadow
	}

	float pdf_brdf = 1;
	if(surface.a >= SPEC_THRESHOLD)
	{
		pdf_brdf = D_GGX(surface.noh, surface.a) * surface.noh /  (4.0 * surface.loh);
		pdf_brdf = max(pdf_brdf, BIAS);
	}
	
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
	ql.quadraticTerm * (dist * dist));

	// balance heuristic
	float mis_weight = 1.0 / (pdf_light + pdf_brdf);

	vec3 Li = ql.color * ql.intensity * attenuation;
	return Li * SampleWorldDirection(surface, L) * mis_weight;
}

vec3 Pointlight_LightSample(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface, float pdf_select, inout uint seed)
{
	vec3 L = normalize(pl.position - surface.position);  

	float dist = distance(pl.position, surface.position);
	if(pl.hasShadow == 1 && PtLights_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, dist) > 0.0){
		return vec3(0.0); // in shadow
	}

	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
	pl.quadraticTerm * (dist * dist));

	vec3 Li = pl.color * pl.intensity * attenuation; 
	return Li * SampleWorldDirection(surface, L) / pdf_select;
}

vec3 Spotlight_LightSample(accelerationStructureEXT topLevelAs, Spotlight sl, Surface surface, float pdf_select, inout uint seed)
{
	vec3 L = normalize(sl.position - surface.position);

	float dist = distance(sl.position, surface.position);
	if(sl.hasShadow == 1 && PtLights_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, dist) > 0.0){
		return vec3(0.0); // in shadow
	}

	float attenuation = 1.0 / (sl.constantTerm + sl.linearTerm * dist + 
	sl.quadraticTerm * (dist * dist));

	// spot effect (soft edges)
	float theta = dot(L, -sl.front);
    float epsilon = (sl.innerCutOff - sl.outerCutOff);
    float spotEffect = clamp((theta - sl.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 Li = sl.color * sl.intensity * attenuation * spotEffect; 
	return Li * SampleWorldDirection(surface, L) / pdf_select;
}

vec3 Dirlight_LightSample(accelerationStructureEXT topLevelAs, Dirlight dl, Surface surface, float pdf_select, inout uint seed)
{
	vec3 L = normalize(-dl.front);

	if(dl.hasShadow == 1 && PtLights_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, 1000000.f) > 0.0){
		return vec3(0.0); // in shadow
	}

	vec3 Li = dl.color * dl.intensity; 
	return Li * SampleWorldDirection(surface, L) / pdf_select;
}

#endif
