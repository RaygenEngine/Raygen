#ifndef pointlight_glsl
#define pointlight_glsl

#include "bsdf.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"

// float tMax      = distance(fragPos, lightPos);	
float ShadowRayQuery(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
{ 
	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAs, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin,
                      direction, tMax);

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
	
	// PERF:
	if(pl.hasShadow == 0){
		return 0.0;
	}

	vec3  L = normalize(pl.position - surface.position); 
	Onb plOrthoBasis = branchlessOnb(L);
	// sample a disk aligned to the L dir

	float dist = distance(surface.position, pl.position);

	float res = 0.f;
	for(uint smpl = 0; smpl < pl.samples; ++smpl){
		// NEXT:
		uint seed = 1;// tea16(uint(gl_FragCoord.y * 1024u + gl_FragCoord.x), pl.samples + smpl);
		vec2 u = rand2(seed);

		vec3 lightSampleV = vec3(uniformSampleDisk(u) * pl.radius, 0.f); 

		vec3 origin = surface.position;

		outOnbSpace(plOrthoBasis, lightSampleV);
		vec3 sampledLpos = pl.position + lightSampleV;
		res+= ShadowRayQuery(topLevelAs, pl.position + lightSampleV, normalize(sampledLpos - surface.position), 0.01f, dist + pl.radius);
	}

	return res / pl.samples;
}

float ShadowRaySimple(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	return ShadowRayQuery(topLevelAs, pl.position, surface.position, 0.001, distance(pl.position, surface.position));
}

vec3 Pointlight_Contribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface, float shadow)
{
	vec3 L = normalize(pl.position - surface.position);
	addSurfaceIncomingLightDirection(surface, L);
	
//	float NoL = Ndot(L); SMATH:
//	if(NoL < BIAS) {
//		discard;
//	}
//
	float dist = length(pl.position - surface.position);
	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
  			     pl.quadraticTerm * (dist * dist));

	vec3 Li = (1.0 - shadow) * pl.color * pl.intensity * attenuation; 

	return DirectLightBRDF(surface)  * Li * surface.NoL;
}

vec3 Pointlight_Contribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	float shadow = ShadowRaySimple(topLevelAs, pl, surface);
	return Pointlight_Contribution(topLevelAs, pl, surface, shadow);

}

vec3 Pointlight_RadiusContribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	float shadow = ShadowRayQueryRadius(topLevelAs, pl, surface);
	return Pointlight_Contribution(topLevelAs, pl, surface, shadow);
}


#endif