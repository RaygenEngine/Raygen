
#ifndef quadlight_glsl
#define quadlight_glsl

#include "bsdf.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "surface.glsl"

bool rayPlaneIntersection(vec3 planeNormal, vec3 planePoint, vec3 rayOrigin, vec3 rayDirection, out float t) 
{ 
    float denom = dot(planeNormal, rayDirection); 
    if (abs(denom) > BIAS) { 

        vec3 toPlaneDir = planePoint - rayOrigin; 
        t = dot(toPlaneDir, planeNormal) / denom; 
        return (t >= 0); 
    } 
 
    return false; 
} 

//bool rayDiskIntersection(vec3 normal, vec3 center, float radius2, vec3 orig, vec3 dir) 
//{ 
//    float t = 0; 
//    if (rayPlaneIntersection(normal, center, orig, dir, t)) { 
//        vec3 p = orig + t * dir; // intersection point with plane 
//        vec3 v = p - center; 
//        float d2 = dot(v, v); 
//        return (d2 <= radius2); 
//     } 
//     return false; 
//} 

bool rayQuadIntersection(vec3 quadCenter, vec3 quadNormal, vec3 quadRight, vec3 quadUp, float quadWidth, float quadHeight, vec3 rayOrigin, vec3 rayDirection, out vec3 p) 
{ 
	// use for positive vectors
	vec3 quadBottopLeftCorner = quadCenter + (-quadWidth / 2.f) * quadRight + (-quadHeight / 2.f) * quadUp;

	float t;
	if (rayPlaneIntersection(quadNormal, quadCenter, rayOrigin, rayDirection, t)) { 
		p = rayOrigin + t * rayDirection; // intersection point with quad's plane 
		vec3 v = p - quadBottopLeftCorner; 
		vec3 vnorm = normalize(v);

		// opposite of either edge vectors
		if(dot(vnorm, quadRight) < 0 || dot(vnorm, quadUp) < 0) {
			return false;
		}

		float len_proj_v_r = dot(v, quadRight);
		float len_proj_v_u = dot(v, quadUp);

		return len_proj_v_r <= quadWidth  && len_proj_v_u <= quadHeight;
	} 
	return false; 
} 

float Quadlight_ShadowRayQuery(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
{ 
	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, 
						  topLevelAs, 
						  gl_RayFlagsTerminateOnFirstHitEXT, 
						  0xFD, 
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

float Quadlight_LightSample(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface, uint seed)
{
	float pdf_area = 1.0 / (ql.width * ql.height);

	vec2 u = rand2(seed) * 2 - 1;
	u.x *= ql.width / 2.f;
	u.y *= ql.height / 2.f;

	vec3 samplePoint =  ql.center + u.x * ql.right + u.y * ql.up;
	
	vec3 L = normalize(samplePoint - surface.position);  
	addIncomingLightDirection(surface, L);

	if(dot(-L, ql.normal) < ql.cosAperture) {
		return 0.0; // if behind light
	}			

	float dist = distance(samplePoint, surface.position);
	if(ql.hasShadow == 1 && Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, dist) > 0.0){
		return 0.0; // in shadow
	}

	float pdf_brdf = 1;
	if(surface.a >= SPEC_THRESHOLD)
	{
		pdf_brdf = D_GGX(surface.noh, surface.a) * surface.noh /  (4.0 * surface.loh);
		pdf_brdf = max(pdf_brdf, BIAS);
	}
	
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
	ql.quadraticTerm * (dist * dist));

	// WIP: check using / (pdf_area + pdf_brdf)
	return attenuation * surface.nol / (pdf_area + pdf_brdf); // MIS
}

float Quadlight_BrdfSample(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface, uint seed)
{
	float pdf_area = 1.0 / (ql.width * ql.height);

	surface.l = reflect(-surface.v);
	cacheSurfaceDots(surface);
	float pdf_brdf = 1;

	if(surface.a >= SPEC_THRESHOLD)
	{
		vec2 u = rand2(seed);
		vec3 H = importanceSampleGGX(u, surface.a);
		surface.l =  reflect(-surface.v, H);
		cacheSurfaceDots(surface);
		pdf_brdf = D_GGX(surface.noh, surface.a) * surface.noh /  (4.0 * surface.loh);
		pdf_brdf = max(pdf_brdf, BIAS);
	}

	vec3 L = surfaceIncidentLightDir(surface);  
	if(dot(-L, ql.normal) < ql.cosAperture) {
		return 0.0;
	}						

	vec3 samplePoint;
	if(!rayQuadIntersection(ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height, surface.position, L, samplePoint)) {
		return 0.0; // no quad intersection
	}

	float dist = distance(samplePoint, surface.position);
	
	if(ql.hasShadow == 1 && Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, dist) > 0.0){
		return 0.0; // in shadow
	}

	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
	ql.quadraticTerm * (dist * dist));

	// WIP: check using / (pdf_area + pdf_brdf)
	return attenuation * surface.nol / (pdf_area + pdf_brdf); // MIS
}

vec3 Quadlight_FastContribution(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	vec3 L = normalize(ql.center - surface.position);
	addIncomingLightDirection(surface, L);

	if(dot(-L, ql.normal) < ql.cosAperture) {
		return vec3(0.0);
	}			

	float dist = distance(ql.center, surface.position);

	if(ql.hasShadow == 1 && Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, dist) > 0.0){
		return vec3(0.0); // in shadow
	}
	
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
  			     ql.quadraticTerm * (dist * dist));

	vec3 Le = ql.color * ql.intensity * attenuation; 

	// point light
	return Le * DirectLightBRDF(surface) * surface.nol;
}




#endif
