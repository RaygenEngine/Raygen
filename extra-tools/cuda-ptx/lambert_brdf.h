#ifndef LAMBERT_BRDF_H
#define LAMBERT_BRDF_H


#include <optixu/optixu_math_namespace.h>

// directions are in surface space

static __device__ float3 Lambert_Eval(
	// Diffuse color
	float3 diffuse_color
)
{
	return diffuse_color / M_PIf;
}

static __device__ float Lambert_Pdf(
	// Outgoing direction
	float3 wo
)
{
	return fabs(wo.y) / M_PIf;
}

/// Lambert BRDF sampling
static __device__ float3 Lambert_Sample(
	// Diffuse color
	float3 diffuse_color,
	// Sample
	float2 sample,
	// Outgoing direction
	float3& wo,
	// PDF at wo
	float& pdf
)
{
	const float3 kd = Lambert_Eval(diffuse_color);

	cosine_sample_hemisphere(sample.x, sample.y, wo);

	pdf = Lambert_Pdf(wo);

	return kd;
}

#endif // LAMBERT_BRDF_H