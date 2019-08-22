#ifndef VRPATHTRACERAUX
#define VRPATHTRACERAUX


#include <optix.h>
#include <optix_math.h>

using namespace optix;
#define FLT_MAX         1e30;
static __device__ __inline__ float3 exp(const float3& x) { return make_float3(exp(x.x), exp(x.y), exp(x.z)); }
static __device__ __inline__ float step(float min, float value) { return value < min ? 0 : 1; }
static __device__ __inline__ float3 mix(float3 a, float3 b, float x) { return a * (1 - x) + b * x; }
static __device__ __inline__ float mix(float a, float b, float x) { return a * (1 - x) + b * x; }

// surface space: normal = (0, 1, 0)
static __device__ __inline__ float3 surface_to_world(const optix::Onb& onb_surface, float3 v)
{
	return make_float3(
		onb_surface.m_tangent.x * v.x + onb_surface.m_normal.x * v.y + onb_surface.m_binormal.x * v.z,
		onb_surface.m_tangent.y * v.x + onb_surface.m_normal.y * v.y + onb_surface.m_binormal.y * v.z,
		onb_surface.m_tangent.z * v.x + onb_surface.m_normal.z * v.y + onb_surface.m_binormal.z * v.z);
}

// surface space: normal = (0, 1, 0)
static __device__ __inline__ float3 world_to_surface(const optix::Onb& onb_surface, float3 v)
{
	return make_float3(dot(v, onb_surface.m_tangent), dot(v, onb_surface.m_normal), dot(v, onb_surface.m_binormal));
}

static __device__ __inline__ float3 schlick(float nDi, const float3& rgb)
{
	float r = fresnel_schlick(nDi, 5, rgb.x, 1);
	float g = fresnel_schlick(nDi, 5, rgb.y, 1);
	float b = fresnel_schlick(nDi, 5, rgb.z, 1);
	return make_float3(r, g, b);
}

static __device__ __inline__ uchar4 make_color(const float3& c)
{
	return make_uchar4(static_cast<unsigned char>(__saturatef(c.z) * 255.99f), /* B */
		static_cast<unsigned char>(__saturatef(c.y) * 255.99f), /* G */
		static_cast<unsigned char>(__saturatef(c.x) * 255.99f), /* R */ 255u); /* A */
}

static __device__ __inline__ float map_range(float input, float input_start,
	float input_end, float output_start, float output_end)
{
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
}

static __device__ __inline__ float rand(float3 co) 
{
	float res = sin(dot(co, make_float3(12.9898, 78.233, 231.23))) * 43758.5453;
	return res - floorf(res);
}

#endif // VRPATHTRACERAUX