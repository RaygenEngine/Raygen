#ifndef surface_glsl
#define surface_glsl

#include "onb.glsl"
#include "random.glsl"
#include "bsdf.glsl"

struct Surface
{
    Onb basis;

    // cached
    vec3 wo; // surface space
    vec3 wi; // surface space
    vec3 h;
    float NoV;
    float NoH;
    float LoH;
    float NoL;
    vec3 position; // p 
    float depth;

    // material
    vec3 albedo;
    vec3 f0;
    vec3 emissive;
    float a;
    float opacity;
    float occlusion;
};

void addSurfaceIncomingLightDirection(inout Surface surface, vec3 L)
{
    surface.wi = toOnbSpaceReturn(surface.basis, L);

    surface.h = normalize(surface.wo + surface.wi); 
    surface.NoV = max(Ndot(surface.wo), BIAS);
    surface.NoH = max(Ndot(surface.h), BIAS);
    surface.LoH = max(dot(surface.wi, surface.h), BIAS);
    surface.NoL = max(Ndot(surface.wi), BIAS);
}

void addSurfaceIncomingLightDirectionInBasis(inout Surface surface, vec3 L)
{
    surface.wi = L;

    surface.h = normalize(surface.wo + surface.wi); 
    surface.NoV = max(Ndot(surface.wo), BIAS);
    surface.NoH = max(Ndot(surface.h), BIAS);
    surface.LoH = max(dot(surface.wi, surface.h), BIAS);
    surface.NoL = max(Ndot(surface.wi), BIAS);
}


void addSurfaceOutgoingLightDirection(inout Surface surface, vec3 V)
{
    surface.wo = toOnbSpaceReturn(surface.basis, V);
}

vec3 reconstructWorldPosition(float depth, vec2 uv, mat4 viewProjInv)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 worldPos = viewProjInv * clipPos;

	return worldPos.xyz / worldPos.w; // return world space pos xyz
}

Surface surfaceFromGBuffer(
    Camera cam,
    sampler2D depthSampler,
    sampler2D normalsSampler, 
    sampler2D albedoOpacitySampler,
    sampler2D f0RoughnessSampler,
    sampler2D emissiveOcclusionSampler,
    vec2 uv)
{
    Surface surface;

    surface.depth = texture(depthSampler, uv).r;

    surface.position = reconstructWorldPosition(surface.depth, uv, cam.viewProjInv);
    
	vec3 normal = texture(normalsSampler, uv).rgb;

    surface.basis = branchlessOnb(normal);

    vec3 V = normalize(cam.position - surface.position);
	addSurfaceOutgoingLightDirection(surface, V);

    // rgb: albedo a: opacity
    vec4 albedoOpacity = texture(albedoOpacitySampler, uv);
    // rgb: f0, a: roughness^2
    vec4 f0Roughness = texture(f0RoughnessSampler, uv);
    // rgb: emissive, a: occlusion
    vec4 emissiveOcclusion = texture(emissiveOcclusionSampler, uv);

    surface.albedo = albedoOpacity.rgb;
    surface.opacity = albedoOpacity.a;

	surface.f0 = f0Roughness.rgb;
	surface.a = f0Roughness.a;

    surface.emissive = emissiveOcclusion.rgb;
    surface.occlusion = emissiveOcclusion.a;

    return surface;
}

vec3 SpecularTerm(Surface surface, vec3 ks)
{
    // CHECK: ks = F should probably be inside the brdfs and omitted here
    if(surface.a < SPEC_THRESHOLD){   
		return ks *  PhongSpecular(surface.NoH, surface.a);
    }

    return ks * CookTorranceGGXSmithCorrelatedSpecular(surface.NoV, surface.NoL, surface.NoH, surface.a);
}

vec3 DiffuseTerm(Surface surface, vec3 kd)
{
    return kd * LambertianDiffuse(surface.albedo);
}

// same Li and L for both brdfs
vec3 DirectLightBRDF(Surface surface)
{
    vec3 ks = F_Schlick(surface.LoH, surface.f0);
    vec3 kd = 1 - ks;
     
    return DiffuseTerm(surface, kd) + SpecularTerm(surface, ks);
}

// PERF:
vec3 SampleSpecularDirection(inout Surface surface, inout uint seed)
{
    vec3 L = reflect(-surface.wo);
    addSurfaceIncomingLightDirectionInBasis(surface, L);
    float pdf = 1;

    if(surface.a >= SPEC_THRESHOLD)
    {
        vec2 u = rand2(seed);
        vec3 H = importanceSampleGGX(u, surface.a);
        L = reflect(-surface.wo, H);
        addSurfaceIncomingLightDirectionInBasis(surface, L);
        pdf = D_GGX(surface.NoH, surface.a) * surface.NoH /  (4.0 * surface.LoH);
        pdf = max(pdf, BIAS);
    }

    vec3 ks = F_Schlick(surface.LoH, surface.f0);
    vec3 brdf_r = SpecularTerm(surface, ks);

    return brdf_r * surface.NoL / pdf;
}



#endif