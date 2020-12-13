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

	vec3 Le = ql.color * ql.intensity * attenuation; 

	// balance heuristic
	float mis_weight = 1.0 / (pdf_light + pdf_brdf);

	return Le * DirectLightBRDF(surface) * surface.nol * mis_weight;
}

#endif
