#ifndef global_h
#define global_h

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef INV_PI
#define INV_PI 0.31830988618379067154f
#endif

#ifndef INV_2PI
#define INV_2PI 0.15915494309189533577f
#endif

#ifndef INV_4PI
#define INV_4PI 0.07957747154594766788f
#endif

#ifndef PI_OVER2
#define PI_OVER2 1.57079632679489661923f
#endif

#ifndef PI_OVER4
#define PI_OVER4 0.78539816339744830961f
#endif

#ifndef SQRT2
#define SQRT2 1.41421356237309504880f
#endif

#ifndef PHI
#define PHI 1.61803398874989484820459f;
#endif

float saturate(float v)
{
    return clamp(v, 0.0, 1.0);
}

vec3 saturate(vec3 v)
{
    return clamp(v, vec3(0.0), vec3(1.0));
}

vec4 SampleCubemapLH(samplerCube cubemap, vec3 RHdirection) 
{
	return texture(cubemap, vec3(RHdirection.x, RHdirection.y, -RHdirection.z));
}

#endif