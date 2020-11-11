#ifndef aabb_glsl
#define aabb_glsl

struct Aabb
{
	vec3 pmin;
	vec3 pmax;
};

Aabb createAabb(vec3 center, float halfsize)
{
	aabb.pmin = center - halfSize;
	aabb.pmax = center + halfSize;

	return aabb;
}

float intersectAabb(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar).y;
};

#endif