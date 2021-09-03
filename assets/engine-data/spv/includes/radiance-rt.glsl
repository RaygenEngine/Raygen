#ifndef radiance_rt_glsl
#define radiance_rt_glsl

#extension GL_EXT_ray_query : require

#include "intersection.glsl"
#include "surface.glsl"

// TODO: this can't handle alpha mask - i.e. it does not know the surface - those types should better use a raytracing pipeline?
float ShadowRayTest(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax, uint cullMask)
{ 
	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, 
						  topLevelAs, 
		                  gl_RayFlagsCullFrontFacingTrianglesEXT,
						  cullMask, 
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

vec3 Pointlight_EstimateDirect(accelerationStructureEXT topLevelAs, Pointlight pl, Surface surface)
{
	vec3 L = normalize(pl.position - surface.position);

	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) { 
		return vec3(0);
	}

	float dist = distance(pl.position, surface.position);
	float shadow = 1 - pl.hasShadow * ShadowRayTest(topLevelAs, surface.position, L, 0.001, dist, 0xFF); // cullMask - none

	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
  			     pl.quadraticTerm * (dist * dist));

	vec3 Li = pl.color * pl.intensity * attenuation * shadow; 
	
	return Li * explicitBRDFcosTheta(surface);
}

#endif
