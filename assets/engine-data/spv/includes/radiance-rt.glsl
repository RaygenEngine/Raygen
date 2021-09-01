#ifndef radiance_rt_glsl
#define radiance_rt_glsl

#extension GL_EXT_ray_query : require

#include "intersection.glsl"
#include "surface.glsl"

// TODO: this can't handle alpha mask
float ShadowRayTest(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
{ 
	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, 
						  topLevelAs, 
		                  gl_RayFlagsCullFrontFacingTrianglesEXT,
						  0xFF, // cullMask - none
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
	float shadow = 1 - pl.hasShadow * ShadowRayTest(topLevelAs, surface.position, L, 0.001, dist);

	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
  			     pl.quadraticTerm * (dist * dist));

	vec3 Li = pl.color * pl.intensity * attenuation * shadow; 
	
	return Li * explicitBRDFcosTheta(surface);
}

// WIP:
//vec3 Quadlight_SpecularContribution(Quadlight ql, Surface surface)
//{
//	if(surface.a < SPEC_THRESHOLD) {
//		return vec3(0);
//	}
//
//	vec3 V = getIncomingDir(surface);
//
//	vec3 L = reflect(-V, surface.basis.normal);
//
//	// we need to rotate the reflection ray to force intersection with quad's plane
//	float t;
//	if (!RayPlaneIntersection(surface.position, L, ql.center, ql.normal, t)) { 
//		return vec3(0);
//		vec3 perp_r_n = L - dot(L, ql.normal) * ql.normal;
//		vec3 pointOnPlane = ql.center + perp_r_n * INF; // SMATH: something big
//		L = normalize(pointOnPlane - surface.position);
//	}
//
//	vec3 p = surface.position + t * L; // intersection point with rect's plane 
//
//	// if point isn't in rectangle, choose the closest that is
//	if(!PointInsideRectangle(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height)) {
//		p = PointRectangleNearestPoint(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height);
//	}
//
//	L = normalize(p - surface.position);
//
//	float cosTheta_o = dot(ql.normal, -L);
//
//	if (cosTheta_o < BIAS) {
//		return vec3(0);
//	}
//
//	addOutgoingDir(surface, L);
//	if(isOutgoingDirPassingThrough(surface)) { 
//		return vec3(0);
//	}
//
//	float dist = distance(p, surface.position);
//
//	return microfacetBRDF(surface);
//}
//
//vec3 Quadlight_DiffuseContribution(Quadlight ql, Surface surface) 
//{
//	return diffuseBRDF(surface);
//}

vec3 Quadlight_EstimateDirect(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	//SMATH:
	vec3 L = normalize(ql.center - surface.position);

	float cosTheta_o = dot(ql.normal, -L);

	if (cosTheta_o < BIAS) {
		return vec3(0); 
	}

	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) { 
		return vec3(0);
	}

	float dist = distance(ql.center, surface.position);
	float shadow = 1 - ShadowRayTest(topLevelAs, surface.position, L, 0.001, dist);

	float attenuation = cosTheta_o / (dist * dist);

	vec3 Li = ql.color * ql.intensity * attenuation * shadow; 
	
	return Li * explicitBRDFcosTheta(surface);
}


#endif
