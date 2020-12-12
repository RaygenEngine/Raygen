#ifndef surface_glsl
#define surface_glsl

#include "bsdf.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"

struct Surface {
    
    // surface
    Onb basis;

    vec3 v;
    vec3 l; 
    vec3 h;
    float nov;
    float noh;
    float loh;
    float nol;


    // material
    vec3 albedo;
    vec3 f0;
    vec3 emissive;
    float a;
    float opacity;
    float occlusion;

    // surface
    vec3 position; 
    float depth;
    vec2 uv;
};

vec3 surfaceIncidentLightDir(Surface surface)
{
    return outOnbSpace(surface.basis, surface.l);
}

vec3 surfaceOutgoingLightDir(Surface surface)
{
    return outOnbSpace(surface.basis, surface.v);
}

// PERF:?
void cacheSurfaceDots(inout Surface surface)
{
    surface.h = normalize(surface.v + surface.l); 
    surface.noh = max(Ndot(surface.h), BIAS);
    surface.loh = max(dot(surface.l, surface.h), BIAS);
    surface.nol = max(Ndot(surface.l), BIAS);
}

void addIncomingLightDirection(inout Surface surface, vec3 L)
{
    surface.l = normalize(toOnbSpace(surface.basis, L));
    cacheSurfaceDots(surface);
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

vec3 reconstructEyePosition(float depth, vec2 uv, mat4 projInv)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 eyePos = projInv * clipPos;

	return eyePos.xyz / eyePos.w; // return eye space pos xyz
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
    surface.v = normalize(toOnbSpace(surface.basis, V));
    surface.nov = max(Ndot(surface.v), BIAS);

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

    surface.uv = uv;

    return surface;
}

#ifndef RAY
Surface surfaceFromGBuffer(
    Camera cam,
    subpassInput depthSampler,
    subpassInput normalsSampler, 
    subpassInput albedoOpacitySampler,
    subpassInput f0RoughnessSampler,
    subpassInput emissiveOcclusionSampler,
    subpassInput use0,
    subpassInput use1,
    vec2 uv)
{
    Surface surface;

    surface.depth = subpassLoad(depthSampler).r;

    surface.position = reconstructWorldPosition(surface.depth, uv, cam.viewProjInv);
    
	vec3 normal = subpassLoad(normalsSampler).rgb;
    surface.basis = branchlessOnb(normal);

    vec3 V = normalize(cam.position - surface.position);
    surface.v = normalize(toOnbSpace(surface.basis, V));
    surface.nov = max(Ndot(surface.v), BIAS);

    // rgb: albedo a: opacity
    vec4 albedoOpacity = subpassLoad(albedoOpacitySampler);
    // rgb: f0, a: roughness^2
    vec4 f0Roughness = subpassLoad(f0RoughnessSampler);
    // rgb: emissive, a: occlusion
    vec4 emissiveOcclusion = subpassLoad(emissiveOcclusionSampler);

    surface.albedo = albedoOpacity.rgb;
    surface.opacity = albedoOpacity.a;

	surface.f0 = f0Roughness.rgb;
	surface.a = f0Roughness.a;

    surface.emissive = emissiveOcclusion.rgb;
    surface.occlusion = emissiveOcclusion.a;

    surface.uv = uv;

    return surface;
}
#endif

vec3 SpecularTerm(Surface surface, vec3 ks)
{
    // CHECK: ks = F should probably be inside the brdfs and omitted here
    if(surface.a < SPEC_THRESHOLD){   
		return ks *  PhongSpecular(surface.noh, surface.a);
    }

    return ks * CookTorranceGGXSmithCorrelatedSpecular(surface.nov, surface.nol, surface.noh, surface.a);
}

vec3 DiffuseTerm(Surface surface, vec3 kd)
{
    return kd * LambertianDiffuse(surface.albedo);
}

// same Li and L for both brdfs
vec3 DirectLightBRDF(Surface surface)
{
    vec3 ks = F_Schlick(surface.loh, surface.f0);
    vec3 kd = 1 - ks;
     
    return DiffuseTerm(surface, kd) + SpecularTerm(surface, ks);
}

// PERF:
vec3 SampleSpecularDirection(inout Surface surface, inout uint seed)
{
    surface.l = reflect(-surface.v);
    cacheSurfaceDots(surface);
    float pdf = 1;

    if(surface.a >= SPEC_THRESHOLD)
    {
        vec2 u = rand2(seed);
        vec3 H = importanceSampleGGX(u, surface.a);
        surface.l =  reflect(-surface.v, H);
        cacheSurfaceDots(surface);
        pdf = D_GGX(surface.noh, surface.a) * surface.noh /  (4.0 * surface.loh);
        pdf = max(pdf, BIAS);
    }

    vec3 ks = F_Schlick(surface.loh, surface.f0);
    vec3 brdf_r = SpecularTerm(surface, ks);

    return brdf_r * surface.nol / pdf;
}

vec3 SampleMirrorDirection(inout Surface surface)
{
    surface.l = reflect(-surface.v);
    cacheSurfaceDots(surface);

    vec3 ks = F_Schlick(surface.loh, surface.f0);
    vec3 brdf_r = SpecularTerm(surface, ks);

    return brdf_r * surface.nol;
}

vec3 SampleDiffuseDirection(inout Surface surface, inout uint seed)
{
    vec2 u = rand2(seed); 
    surface.l = cosineSampleHemisphere(u);
    cacheSurfaceDots(surface);

    float pdf = surface.nol * INV_PI;

	vec3 kd = 1 - F_Schlick(surface.loh, surface.f0);
    vec3 brdf_d = DiffuseTerm(surface, kd);
	
	return brdf_d * surface.nol / pdf;
}


#endif
