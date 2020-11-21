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

float intersectionDistanceAabb(vec3 boxMin, vec3 boxMax, vec3 rayOrigin, vec3 rayDirection) {
    vec3 tMin = (boxMin - rayOrigin) / rayDirection;
    vec3 tMax = (boxMax - rayOrigin) / rayDirection;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar).y;
};

#endif