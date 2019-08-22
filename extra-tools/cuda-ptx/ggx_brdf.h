#ifndef GGX_BRDF_H
#define GGX_BRDF_H


#include <optixu/optixu_math_namespace.h>

using namespace optix;

#define DENOM_EPS 1e-8f


static __device__ __inline__ float MicrofacetDistribution_GGX_D(float roughness, float3 m)
{
	float ndotm = fabs(m.y);
	float ndotm2 = ndotm * ndotm;
	float sinmn = sqrt(1.f - clamp(ndotm * ndotm, 0.f, 1.f));
	float tanmn = ndotm > DENOM_EPS ? sinmn / ndotm : 0.f;
	float a2 = roughness * roughness;
	float denom = (M_PIf * ndotm2 * ndotm2 * (a2 + tanmn * tanmn) * (a2 + tanmn * tanmn));
	return denom > DENOM_EPS ? (a2 / denom) : 1.f;
}

static __device__ __inline__ float MicrofacetDistribution_GGX_G1(float roughness, float3 v, float3 m)
{
	float ndotv = fabs(v.y);
	float mdotv = fabs(dot(m, v));

	float sinnv = sqrt(1.f - clamp(ndotv * ndotv, 0.f, 1.f));
	float tannv = ndotv > DENOM_EPS ? sinnv / ndotv : 0.f;
	float a2 = roughness * roughness;
	return 2.f / (1.f + sqrt(1.f + a2 * tannv * tannv));
}

// Shadowing function also depends on microfacet distribution
static __device__ __inline__ float MicrofacetDistribution_GGX_G(float roughness, float3 wi, float3 wo, float3 wh)
{
	return MicrofacetDistribution_GGX_G1(roughness, wi, wh) * MicrofacetDistribution_GGX_G1(roughness, wo, wh);
}

static __device__ __inline__ float3 Fresnel_Schlick(float VoH, const float3& f0) {
	return f0 + (make_float3(1.0) - f0) * pow(1.0 - VoH, 5.0);
}

static __device__ __inline__ float3 MicrofacetGGX_Evaluate(
	// Rougness
	float roughness,
	// Reflectivity
	float3 ks,
	// Halfway vector
	float3 wh,
	// Incoming direction
	float3 wi,
	// Outgoing direction
	float3 wo
)
{
	// Incident and reflected zenith angles
	float costhetao = fabs(wo.y);
	float costhetai = fabs(wi.y);

	// Calc halfway vector
	//float3 wh = normalize(wi + wo);

	float denom = (4.f * costhetao * costhetai);

	return denom > DENOM_EPS ? ks * MicrofacetDistribution_GGX_G(roughness, wi, wo, wh) * MicrofacetDistribution_GGX_D(roughness, wh) / denom : make_float3(0.f);

}

// PDF of the given direction
static __device__ __inline__ float MicrofacetDistribution_GGX_GetPdf(
	// Rougness
	float roughness,
	// Halfway vector
	float3 wh,
	// Outgoing direction
	float3 wo
)
{
    float mpdf = MicrofacetDistribution_GGX_D(roughness, wh) * fabs(wh.y);
    // See Humphreys and Pharr for derivation
    float denom = (4.f * fabs(dot(wo, wh)));

    return denom > DENOM_EPS ? mpdf / denom : 0.f;
}

// In surface space
// Sample the distribution 
static __device__ __inline__ void MicrofacetDistribution_GGX_SampleHalfwayVector(
	// Incoming direction
	const float3& wi,
	// Roughness
	float roughness,
	// Sample
	float2 sample,
	// Halfway direction
	float3& wh,
	// Outgoing direction
	float3& wo,
	// PDF
	float& pdf
)
{
	float r1 = sample.x;
	float r2 = sample.y;

	// Sample halfway vector first, then reflect wi around that
	float theta = atan2(roughness * sqrt(r1), sqrt(1.f - r1));
	float costheta = cos(theta);
	float sintheta = sin(theta);

	// phi = 2*PI*ksi2
	float phi = 2.f * M_PIf * r2;
	float cosphi = cos(phi);
	float sinphi = sin(phi);

	// Calculate halfway
	wh = make_float3(sintheta * cosphi, costheta, sintheta * sinphi);

	// Calculate outgoing
	wo = optix::reflect(-wi, wh);

	// Calculate pdf
	pdf = MicrofacetDistribution_GGX_GetPdf(roughness, wh, wo);
}



static __device__ __inline__ float D_GGX(float NoH, float a) {
	float a2 = a * a;
	float f = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (M_PIf * f * f);
}

static __device__ __inline__ float3 F_Schlick(float VoH, float3 f0) {
	return f0 + (make_float3(1.0) - f0) * pow(1.0 - VoH, 5.0);
}

static __device__ __inline__ float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
	float a2 = a * a;
	float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
	float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
	return 0.5 / (GGXV + GGXL);
}

static __device__ __inline__ float3 BRDF(float3 v, float3 l, float3 n, float roughness, float3 f0) {
	float3 h = normalize(v + l);

	float NoV = abs(dot(n, v)) + 1e-5;
	float NoL = clamp(dot(n, l), 0.0, 1.0);
	float NoH = clamp(dot(n, h), 0.0, 1.0);
	float LoH = clamp(dot(l, h), 0.0, 1.0);

	float D = D_GGX(NoH, roughness);
	float3  F = F_Schlick(LoH, f0);
	float V = V_SmithGGXCorrelated(NoV, NoL, roughness);

	// specular BRDF
	return (D * V) * F;

	// diffuse BRDF
	//float3 Fd = diffuseColor * Fd_Lambert();

	// apply lighting...
}

static __device__ __inline__ void GetTangentAndBitangent(float3 normal, float3& tangent, float3& bitangent)
{
	// calculate tangent, bitangent
	tangent = cross(normal, make_float3(0.0f, 1.0f, 0.0f));
	if (dot(tangent, tangent) < 1.e-3f)
		tangent = cross(normal, make_float3(1.0f, 0.0f, 0.0f));
	tangent = normalize(tangent);
	bitangent = normalize(cross(normal, tangent));
}

static __device__ __inline__ float3 GetNewSamplePositionNDFSampling(float& out_inv_pdf, float3 I,
	float currect_vertex_roughness, float3 current_vertex_normal,
	float2 sample)
{
	float tantheta2;
	float costheta;
	float sintheta;

	float tantheta = currect_vertex_roughness * sqrt(sample.x) / sqrt(1 - sample.x);
	tantheta2 = tantheta * tantheta;
	costheta = sqrt(1.0f / (1.0f + tantheta2));
	sintheta = sqrt(1.0f - costheta * costheta);
	
	float phi = 2.f*M_PIf * sample.y;
	float _x = cos(phi) * sintheta;
	float _y = sin(phi) * sintheta;
	float _z = costheta;

	//float3 right, front;
	//GetTangentAndBitangent(current_vertex_normal, right, front);

	//optix::Onb onb_local(current_vertex_normal);
	
	float3 right, front;
	GetTangentAndBitangent(current_vertex_normal, right, front);

	float3 H = normalize(_x * right + _y * front + _z * current_vertex_normal);

	float HI = dot(H, -I);

	float HN = dot(H, current_vertex_normal);
	float HN_clamped = max(0.0f, HN);

	float3 sample_dir = optix::reflect(I, H);

	float pdf = D_GGX(HN, currect_vertex_roughness) * abs(HN) * 0.25f / abs(HI);

	out_inv_pdf = 1.0f / max(DENOM_EPS, pdf);

	return sample_dir;
}


static __device__ __inline__ float3 GlossyBRDF(float3 sample_dir, float3 view_dir,
	float3 normal, float roughness, float3 f0)
{
	float3  H = view_dir + sample_dir;
	H = normalize(H);

	float NL = dot(normal, sample_dir);
	float NV = dot(normal, view_dir);
	float HN = dot(H, normal);
	float HV = dot(H, view_dir);

	float D = D_GGX(HN, roughness);
	float3  F = F_Schlick(HV, f0);
	float V = V_SmithGGXCorrelated(NV, NL, roughness);

	float3 glossy_brdf = (NV * NL > 0.0f) ? D * F * V * 0.25f*(1 / (DENOM_EPS + abs(NV) * abs(NL))) : make_float3(0.f, 0.f, 0.f);

	return glossy_brdf * max(0.0f, NL);
}



#endif // GGX_BRDF_H