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

// Representative point method
vec3 Arealight_EstimateDirectNoLightAttenuation(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	vec3 V = getIncomingDir(surface);

	vec3 L = reflect(-V, surface.basis.normal);

	// we need to rotate the reflection ray to force intersection with quad's plane
	float t;
	if (!RayPlaneIntersection(surface.position, L, ql.center, ql.normal, t)) {
		vec3 perp_r_n = L - dot(L, ql.normal) * ql.normal;
		vec3 pointOnPlane = ql.center + perp_r_n * INF;
		L = normalize(pointOnPlane - surface.position);
	}

	vec3 p = surface.position + t * L; // intersection point with rect's plane 

	// if point isn't in rectangle, choose the closest that is
	if (!PointInsideRectangle(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height)) {
		p = PointRectangleNearestPoint(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height);
	}

	L = normalize(p - surface.position);

	addOutgoingDir(surface, L);
	if (isOutgoingDirPassingThrough(surface)) {
		return vec3(0);
	}

	float dist = distance(p, surface.position);

	float shadow = 1 - ql.hasShadow * ShadowRayTest(topLevelAs, surface.position, L, 0.001, dist, 0xFD); // cullMask - quadlights

	vec3 Li = ql.color * ql.intensity * shadow; // missing smooth shadow and attenuation, i.e. arealightShadowing factor

	return Li * explicitBRDF(surface);
}

// Representative point method
vec3 Arealight_EstimateDirectHackLightAttenuation(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	vec3 V = getIncomingDir(surface);

	vec3 L = reflect(-V, surface.basis.normal);

	// we need to rotate the reflection ray to force intersection with quad's plane
	float t;
	if (!RayPlaneIntersection(surface.position, L, ql.center, ql.normal, t)) {
		vec3 perp_r_n = L - dot(L, ql.normal) * ql.normal;
		vec3 pointOnPlane = ql.center + perp_r_n * INF;
		L = normalize(pointOnPlane - surface.position);
	}

	vec3 p = surface.position + t * L; // intersection point with rect's plane 

	// if point isn't in rectangle, choose the closest that is
	if (!PointInsideRectangle(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height)) {
		p = PointRectangleNearestPoint(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height);
	}

	L = normalize(p - surface.position);

	float cosTheta_o = dot(ql.normal, -L);

	if (cosTheta_o < BIAS) {
		return vec3(0);
	}

	addOutgoingDir(surface, L);
	if (isOutgoingDirPassingThrough(surface)) {
		return vec3(0);
	}

	float dist = distance(p, surface.position);

	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist +
		ql.quadraticTerm * (dist * dist));

	float shadow = 1 - ql.hasShadow * ShadowRayTest(topLevelAs, surface.position, L, 0.001, dist, 0xFD); // cullMask - quadlights

	vec3 Li = ql.color * ql.intensity * shadow * attenuation; // missing smooth shadow and attenuation, i.e. arealightShadowing factor

	return Li * explicitBRDFcosTheta(surface);
}


#endif
