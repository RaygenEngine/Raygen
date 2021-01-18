#ifndef intersection_glsl
#define intersection_glsl

// SMATH: header

float RaySphereIntersection(vec3 orig, vec3 dir, vec3 center, float radius)
{
	vec3 oc = orig - center;
	float a = dot(dir, dir);
	float b = 2.0 * dot(oc, dir);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0.0) {
		return -1.0;
	}
	else {
		float numerator = -b - sqrt(discriminant);
		if (numerator
			> 0.0) { return numerator / (2.0 * a); }

		numerator = -b + sqrt(discriminant);
		if (numerator
			> 0.0) { return numerator / (2.0 * a); }
		else {
			return -1;
		}
	}
}

bool RayPlaneIntersection(vec3 rayOrigin, vec3 rayDirection, vec3 planeNormal, vec3 planePoint, out float t) 
{ 
    float denom = dot(planeNormal, rayDirection); 
    if (abs(denom) > BIAS) { 

        vec3 toPlaneDir = planePoint - rayOrigin; 
        t = dot(toPlaneDir, planeNormal) / denom; 
        return (t >= 0); 
    } 
 
    return false; 
} 

bool RayDiskIntersection(vec3 orig, vec3 dir, vec3 normal, vec3 center, float radius2) 
{ 
    float t = 0; 
    if (RayPlaneIntersection(orig, dir, normal, center, t)) { 
        vec3 p = orig + t * dir; // intersection point with plane 
        vec3 v = p - center; 
        float d2 = dot(v, v); 
        return (d2 <= radius2); 
     } 
     return false; 
} 

bool QuadRayIntersection(vec3 rayOrigin, vec3 rayDirection, vec3 quadCenter, vec3 quadNormal, vec3 quadRight, vec3 quadUp, float quadWidth, float quadHeight, out vec3 p) 
{ 
	// use for positive vectors
	vec3 quadBottopLeftCorner = quadCenter + (-quadWidth / 2.f) * quadRight + (-quadHeight / 2.f) * quadUp;

	float t;
	if (RayPlaneIntersection(rayOrigin, rayDirection, quadNormal, quadCenter, t)) { 
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

#endif
