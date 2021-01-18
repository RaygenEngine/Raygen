#ifndef pointlight_glsl
#define pointlight_glsl

#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "shading-math.glsl"
#include "shadowmap.glsl"
#include "surface.glsl"

float Pointlight_ShadowRayQuery(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
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

vec3 Pointlight_Sample(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface, vec3 L)
{
	float dist = distance(pl.position, surface.position);
	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
  			     pl.quadraticTerm * (dist * dist));

	vec3 Li = pl.color * pl.intensity * attenuation;  
	return Li * explicitBRDFcosTheta(surface, L);
}

vec3 Pointlight_MultipleSamples(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	float inv_pdf = PI * pl.radius * pl.radius;

	vec3 plFront = normalize(pl.position - surface.position); 
	Onb lightOrthoBasis = branchlessOnb(plFront);

	// TODO: get res from global desc set -> render scale also should be factored here
	uint seed = tea16(uint(surface.uv.y * 2160 * 4096 + surface.uv.x * 4096), pl.samples);

	vec3 res = vec3(0);


	for(uint smpl = 0; smpl < pl.samples; ++smpl){

		vec2 u = rand2(seed);

		vec3 samplePoint = vec3(uniformSampleDisk(u) * pl.radius, 0.f); 
		samplePoint = pl.position + outOnbSpace(lightOrthoBasis, samplePoint);

		vec3 L = normalize(samplePoint - surface.position);  

		float shadow = pl.hasShadow != 0 ? Pointlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, distance(pl.position, surface.position)) : 0;

		res += Pointlight_Sample(topLevelAs, pl, surface, L) * (1 - shadow) * inv_pdf;
	}

	return res / float(pl.samples);
}

vec3 Pointlight_FastContribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	vec3 L = normalize(pl.position - surface.position);

	float shadow = pl.hasShadow != 0 ? Pointlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, distance(pl.position, surface.position)) : 0;

	// single sample
	return Pointlight_Sample(topLevelAs, pl, surface, L) * (1 - shadow);
}

vec3 Pointlight_SmoothContribution(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	return Pointlight_MultipleSamples(topLevelAs, pl, surface);
}


#endif
