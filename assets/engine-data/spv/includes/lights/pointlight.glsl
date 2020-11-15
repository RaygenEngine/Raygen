#ifndef pointlight_glsl
#define pointlight_glsl

#include "bsdf.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "surface.glsl"

float ShadowRayQuery(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
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

float ShadowRayQueryRadius(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	vec3  L = normalize(pl.position - surface.position); 
	Onb lightOrthoBasis = branchlessOnb(L);
	// sample a disk aligned to the L dir

	float dist = distance(surface.position, pl.position);

	float res = 0.f;
	for(uint smpl = 0; smpl < pl.samples; ++smpl){
		// WIP: seed
		uint seed = tea16(uint(surface.uv.y * 2160 * 4096 + surface.uv.x * 4096), pl.samples + smpl);
		vec2 u = rand2(seed);

		vec3 lightSampleV = vec3(uniformSampleDisk(u) * pl.radius, 0.f); 

		outOnbSpace(lightOrthoBasis, lightSampleV);

		vec3 sampledLpos = pl.position + lightSampleV;
		vec3  direction = normalize(sampledLpos - surface.position);  

		res += ShadowRayQuery(topLevelAs, surface.position, direction, 0.01, dist + pl.radius);
	}

	return res / pl.samples;
}

float ShadowRaySimple(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	vec3 L = normalize(pl.position - surface.position);
	return ShadowRayQuery(topLevelAs, surface.position, L, 0.01, distance(pl.position, surface.position));
}

vec3 Pointlight_Contribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface, float shadow)
{
	vec3 L = normalize(pl.position - surface.position);
	addIncomingLightDirection(surface, L);

	float dist = distance(pl.position, surface.position);
	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
  			     pl.quadraticTerm * (dist * dist));

	vec3 Li = (1.0 - shadow) * pl.color * pl.intensity * attenuation; 

	return DirectLightBRDF(surface)  * Li * surface.nol;
}

vec3 Pointlight_FastContribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	float shadow = pl.hasShadow != 0 ? ShadowRaySimple(topLevelAs, pl, surface) : 0;
	return Pointlight_Contribution(topLevelAs, pl, surface, shadow);

}

vec3 Pointlight_SmoothContribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	float shadow = pl.hasShadow != 0 ? ShadowRayQueryRadius(topLevelAs, pl, surface) : 0;
	return Pointlight_Contribution(topLevelAs, pl, surface, shadow);
}


#endif
