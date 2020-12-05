#ifndef quadlight_glsl
#define quadlight_glsl

#include "bsdf.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "surface.glsl"

float Quadlight_ShadowRayQuery(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
{ 
	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, 
						  topLevelAs, 
						  gl_RayFlagsTerminateOnFirstHitEXT, 
						  0xFF, 
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

vec3 Quadlight_Sample(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	float dist = distance(ql.position, surface.position);
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
  			     ql.quadraticTerm * (dist * dist));

	vec3 Le = ql.color * ql.intensity * attenuation; 

	return Le * DirectLightBRDF(surface) * surface.nol;
}

vec3 Quadlight_MultipleSamples(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	float inv_pdf = ql.scaleX * ql.scaleY;

	uint seed = tea16(uint(surface.uv.y * 2160 * 4096 + surface.uv.x * 4096), ql.samples);

	Onb lightOrthoBasis = branchlessOnb(ql.front);

	vec3 res = vec3(0);
	for(int smpl = 0; smpl < ql.samples; ++smpl) {

		vec2 u = rand2(seed) * 2 - 1;
		u.x *= ql.scaleX;
		u.y *= ql.scaleY;

		vec3 samplePoint = ql.position + outOnbSpace(lightOrthoBasis, vec3(u, 0));
	
		vec3 L = normalize(samplePoint - surface.position);  
		addIncomingLightDirection(surface, L);

		if(dot(-L, ql.front) < 0) {
			continue; // if behind light
		}			

		float shadow = ql.hasShadow != 0 ? Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, distance(ql.position, surface.position)) : 0;

		res += Quadlight_Sample(topLevelAs, ql, surface) * (1 - shadow) * inv_pdf;
	}

	return res / float(ql.samples);
}

vec3 Quadlight_FastContribution(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	vec3 L = normalize(ql.position - surface.position);
	addIncomingLightDirection(surface, L);

	float shadow = ql.hasShadow != 0 ? Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, distance(ql.position, surface.position)) : 0;

	// single sample - like point light
	return Quadlight_Sample(topLevelAs, ql, surface) * (1 - shadow);
}

vec3 Quadlight_SmoothContribution(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	return Quadlight_MultipleSamples(topLevelAs, ql, surface);
}


#endif
