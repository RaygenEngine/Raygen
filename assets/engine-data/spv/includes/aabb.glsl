#ifndef aabb_glsl
#define aabb_glsl

struct Aabb
{
	vec3 pmin;
	vec3 pmax;
};

Aabb createAabb(vec3 center, float halfsize)
{
    Aabb aabb;
	aabb.pmin = center - vec3(halfsize);
	aabb.pmax = center + vec3(halfsize);

	return aabb;
}

// not necessarily cube
Aabb createAabb(vec3 pmin, vec3 pmax)
{
    Aabb aabb;
	aabb.pmin = pmin;
	aabb.pmax = pmax;

	return aabb;
}

vec3 getAabbCenter(Aabb aabb)
{
    return (aabb.pmin + aabb.pmax) / 2.f;
}

vec3 getAabbExtent(Aabb aabb)
{
    return (aabb.pmin - aabb.pmax) / 2.f;
}

bool containsPointAabb(Aabb aabb, vec3 p)
{
// SMATH: =
	return p.x >= aabb.pmin.x && p.x <= aabb.pmax.x
		&& p.y >= aabb.pmin.y && p.y <= aabb.pmax.y
		&& p.z >= aabb.pmin.z && p.z <= aabb.pmax.z;
}

// distance from inside point
float distanceFromInside(Aabb aabb, vec3 p) 
{

    return min(min(min(p.x - aabb.pmin.x, aabb.pmax.x - p.x), 
 	               min(p.y - aabb.pmin.y, aabb.pmax.y - p.y)),
 	               min(p.z - aabb.pmin.z, aabb.pmax.z - p.z));

}

float distance2FromInside(Aabb aabb, vec3 p) 
{

    return distanceFromInside(aabb, p) * distanceFromInside(aabb, p);
}

// distance from outside point
float distanceFromOutside(Aabb aabb, vec3 p) 
{
  float dx = max(max(aabb.pmin.x - p.x, 0.0), p.x - aabb.pmax.x);
  float dy = max(max(aabb.pmin.y - p.y, 0.0), p.y - aabb.pmax.y);
  float dz = max(max(aabb.pmin.z - p.z, 0.0), p.z - aabb.pmax.z);
  
  return sqrt(dx*dx + dy*dy + dz*dz);
}

float distance2FromOutside(Aabb aabb, vec3 p) 
{
  float dx = max(max(aabb.pmin.x - p.x, 0.0), p.x - aabb.pmax.x);
  float dy = max(max(aabb.pmin.y - p.y, 0.0), p.y - aabb.pmax.y);
  float dz = max(max(aabb.pmin.z - p.z, 0.0), p.z - aabb.pmax.z);
  
  return dx*dx + dy*dy + dz*dz;
}

// distance from everywhere
float distance(Aabb aabb, vec3 p) 
{
    return containsPointAabb(aabb, p) ? distanceFromInside(aabb, p) : distanceFromOutside(aabb, p);
}

float distance2(Aabb aabb, vec3 p) 
{
    return containsPointAabb(aabb, p) ? distance2FromInside(aabb, p) : distance2FromOutside(aabb, p);
}

// returns distance from ray origin to aabb
float intersectionDistanceAabb(Aabb aabb, vec3 rayOrigin, vec3 rayDirection) {
    vec3 tMin = (aabb.pmin - rayOrigin) / rayDirection;
    vec3 tMax = (aabb.pmax - rayOrigin) / rayDirection;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar).y;
};

#endif
