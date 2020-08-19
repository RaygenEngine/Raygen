#ifndef global_h
#define global_h

#ifndef PI
#define PI 3.14159265358979323846f
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

vec4 SampleCubemapLH(samplerCube cubemap, vec3 RHdirection) {
	return texture(cubemap, vec3(RHdirection.x, RHdirection.y, -RHdirection.z));
}

void createOnb(const vec3 normal, inout vec3 T, inout vec3 B, inout vec3 N)
{
	N = normalize(normal);
	T = cross(N, vec3(0.0f, 1.0f, 0.0f));
	if(abs(T.x) < 0.001f && abs(T.y) < 0.001f && abs(T.z) < 0.001f)
		T = cross(N, vec3(1.0f, 0.0f, 0.0f));
	T = normalize(T);
	B = cross(N, T);
}

#endif