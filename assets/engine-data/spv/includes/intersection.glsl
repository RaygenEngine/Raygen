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

bool RayPlaneIntersection(vec3 rayOrigin, vec3 rayDirection, vec3 planePoint, vec3 planeNormal, out float t) 
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
    if (RayPlaneIntersection(orig, dir, center, normal, t)) { 
        vec3 p = orig + t * dir; // intersection point with plane 
        vec3 v = p - center; 
        float d2 = dot(v, v); 
        return (d2 <= radius2); 
     } 
     return false; 
} 

bool PointInsideRectangle(vec3 point, vec3 rectCenter, vec3 rectNormal, vec3 rectRight, vec3 rectUp, float rectWidth, float rectHeight) 
{
	vec3 rectBottopLeftCorner = rectCenter + (-rectWidth / 2.f) * rectRight + (-rectHeight / 2.f) * rectUp;

	vec3 v = point - rectBottopLeftCorner; 

	float len_proj_v_r = dot(v, rectRight);
	float len_proj_v_u = dot(v, rectUp);

	// opposite of either edge vectors
	if(len_proj_v_r < 0 || len_proj_v_u < 0) {
		return false;
	}

	return len_proj_v_r <= rectWidth && len_proj_v_u <= rectHeight;
}

bool RayRectangleIntersection(vec3 rayOrigin, vec3 rayDirection, vec3 rectCenter, vec3 rectNormal, vec3 rectRight, vec3 rectUp, float rectWidth, float rectHeight, out vec3 p) 
{ 
	float t;
	if (RayPlaneIntersection(rayOrigin, rayDirection, rectCenter, rectNormal, t)) { 
		p = rayOrigin + t * rayDirection; // intersection point with rect's plane 

		return PointInsideRectangle(p, rectCenter, rectNormal, rectRight, rectUp, rectWidth, rectHeight);
	} 
	return false; 
} 

vec3 PointLineSegmentNearestPoint(vec3 point, vec3 lineA, vec3 lineB)
{
	vec3 AB = lineB - lineA;

	float t = dot(point - lineA, AB) / dot(AB, AB); // signed projection length

	return lineA + saturate(t) * AB;
}

vec3 PointRectangleNearestPoint(vec3 point, vec3 rectCenter, vec3 rectNormal, vec3 rectRight, vec3 rectUp, float rectWidth, float rectHeight) 
{
	vec3 rectBottopLeftCorner = rectCenter + (-rectWidth / 2.f) * rectRight + (-rectHeight / 2.f) * rectUp;
	vec3 rectTopRightCorner = rectCenter + (rectWidth / 2.f) * rectRight + (rectHeight / 2.f) * rectUp;

	vec3 selectedPoint = PointLineSegmentNearestPoint(point, rectBottopLeftCorner, rectBottopLeftCorner + rectRight * rectWidth);
	float selectedSqrDist = dot(selectedPoint - point, selectedPoint - point);

	vec3 candidatePoint = PointLineSegmentNearestPoint(point, rectBottopLeftCorner, rectBottopLeftCorner + rectUp * rectHeight);
	float candidateSqrDist = dot(candidatePoint - point, candidatePoint - point);

	if(selectedSqrDist > candidateSqrDist) {
		selectedPoint = candidatePoint;
		selectedSqrDist = candidateSqrDist;
	}

	candidatePoint = PointLineSegmentNearestPoint(point, rectTopRightCorner, rectTopRightCorner - rectRight * rectWidth);
	candidateSqrDist = dot(candidatePoint - point, candidatePoint - point);

	if(selectedSqrDist > candidateSqrDist) {
		selectedPoint = candidatePoint;
		selectedSqrDist = candidateSqrDist;
	}

	candidatePoint = PointLineSegmentNearestPoint(point, rectTopRightCorner, rectTopRightCorner - rectUp * rectHeight);
	candidateSqrDist = dot(candidatePoint - point, candidatePoint - point);

	if(selectedSqrDist > candidateSqrDist) {
		selectedPoint = candidatePoint;
	}

	return selectedPoint;
}

#endif
