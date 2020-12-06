
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
	float dist = distance(ql.center, surface.position);
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
  			     ql.quadraticTerm * (dist * dist));

	vec3 Le = ql.color * ql.intensity * attenuation; 

	return Le * DirectLightBRDF(surface) * surface.nol;
}

vec3 Quadlight_MultipleSamples1(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	float pdf_l = 1.0 / (ql.width * ql.height);

	uint seed = tea16(uint(surface.uv.y * 2160 * 4096 + surface.uv.x * 4096), ql.samples);

	vec3 res = vec3(0);
	for(int smpl = 0; smpl < ql.samples; ++smpl) {

		vec2 u = rand2(seed) * 2 - 1;
		u.x *= ql.width / 2.f;
		u.y *= ql.height / 2.f;

		vec3 samplePoint =  ql.center + u.x * ql.right + u.y * ql.up;
	
		vec3 L = normalize(samplePoint - surface.position);  
		addIncomingLightDirection(surface, L);

		if(dot(-L, ql.normal) < 0) {
			continue; // if behind light
		}			

		float dist = distance(samplePoint, surface.position);
		if(ql.hasShadow == 1 && Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, dist) > 0.0){
			continue; // in shadow
		}

		float pdf_f = 1;
		if(surface.a >= SPEC_THRESHOLD)
		{
			pdf_f = D_GGX(surface.noh, surface.a) * surface.noh /  (4.0 * surface.loh);
			pdf_f = max(pdf_f, BIAS);
		}

		float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
		ql.quadraticTerm * (dist * dist));

		vec3 Le = ql.color * ql.intensity * attenuation; 
		res += Le * DirectLightBRDF(surface) * surface.nol / (pdf_l + pdf_f);
	}

	return res / float(ql.samples);
}

vec3 Quadlight_MultipleSamples2(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	float pdf_l = 1.0 / (ql.width * ql.height);

	uint seed = tea16(uint(surface.uv.y * 2160 * 4096 + surface.uv.x * 4096), ql.samples);

	vec3 res = vec3(0);
	for(int smpl = 0; smpl < ql.samples; ++smpl) {

	    surface.l = reflect(-surface.v);
		cacheSurfaceDots(surface);
		float pdf_f = 1;

		if(surface.a >= SPEC_THRESHOLD)
		{
			vec2 u = rand2(seed);
			vec3 H = importanceSampleGGX(u, surface.a);
			surface.l =  reflect(-surface.v, H);
			cacheSurfaceDots(surface);
			pdf_f = D_GGX(surface.noh, surface.a) * surface.noh /  (4.0 * surface.loh);
			pdf_f = max(pdf_f, BIAS);
		}

		vec3 L = surfaceIncidentLightDir(surface);  
		if(dot(-L, ql.normal) < 0) {
			continue; // if behind light
		}			

		vec3 p;
		if(!rayQuadIntersection(ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height, surface.position, L, p)) {
			continue; // no quad intersection
		}

		if(ql.hasShadow == 1 && Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, distance(p, surface.position)) > 0.0){
			continue; // in shadow
		}

		vec3 ks = F_Schlick(surface.loh, surface.f0);
		vec3 brdf_r = SpecularTerm(surface, ks);

	    float dist = distance(p, surface.position);
		float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
  	    ql.quadraticTerm * (dist * dist));

		vec3 Le = ql.color * ql.intensity * attenuation; 

		res += Le * brdf_r * surface.nol / (pdf_l + pdf_f);
	}

	return res / float(ql.samples);
}


vec3 Quadlight_FastContribution(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	vec3 L = normalize(ql.center - surface.position);
	addIncomingLightDirection(surface, L);

	float shadow = ql.hasShadow != 0 ? Quadlight_ShadowRayQuery(topLevelAs, surface.position, L, 0.01, distance(ql.center, surface.position)) : 0;

	// single sample - like point light
	return Quadlight_Sample(topLevelAs, ql, surface) * (1 - shadow);
}

vec3 Quadlight_SmoothContribution(accelerationStructureEXT topLevelAs, Quadlight ql, Surface surface)
{
	return Quadlight_MultipleSamples1(topLevelAs, ql, surface) + Quadlight_MultipleSamples2(topLevelAs, ql, surface);
}


#endif
