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

void cacheSurfaceDots(inout Surface surface)
{
    surface.h = normalize(surface.v + surface.l); 
    surface.noh = max(Ndot(surface.h), BIAS);
    surface.loh = max(dot(surface.l, surface.h), BIAS);
    surface.nol = max(Ndot(surface.l), BIAS);
}

// caches as if the light direction hits the frontface
void cacheBackSurfaceDots(inout Surface surface)
{
    vec3 l = surface.l;
    l.z *= -1;

    surface.h = normalize(surface.v + l); 
    surface.noh = max(Ndot(surface.h), BIAS);
    surface.loh = max(dot(l, surface.h), BIAS);
    surface.nol = max(Ndot(l), BIAS);
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

// SMATH:
float microfacetBrdfNoL(Surface surface) 
{
    float NDF = D_GGX(surface.noh, surface.a);
    float G = G_SmithSchlickGGX(surface.nol, surface.nov, surface.a);

    float numerator = NDF * G;
    float denominator = 4.0 * surface.nov; // omit nol

    return numerator / max(denominator, 0.001); 
}

float microfacetBtdfNoL(Surface surface, float iorRation) 
{
    float NDF = D_GGX(surface.noh, surface.a);
    float G = G_SmithSchlickGGX(surface.nol, surface.nov, surface.a);

    // SMATH:
    float numerator = NDF * G * surface.loh * surface.loh;
    float denominator =  (surface.loh + iorRation * surface.loh); 
    denominator *= denominator;
    denominator *= surface.nov;// omit nol

    return numerator / max(denominator, 0.001); 
}

// SMATH:
vec3 SampleWorldDirection(inout Surface surface, vec3 L)
{
    addIncomingLightDirection(surface, L);

    vec3 ks = F_Schlick(surface.loh, surface.f0);
    vec3 kd = 1 - ks;

    vec3 brdf_d = DisneyDiffuse(surface.nol, surface.nov, surface.loh, surface.a, surface.albedo);

    float brdfr_nol = surface.a < SPEC_THRESHOLD ?  BlinnPhongSpecular(surface.noh, surface.a) : microfacetBrdfNoL(surface);
    
    return kd * brdf_d * surface.nol + ks * brdfr_nol;
}

// returns specular transmission brdf * nol
float SampleSpecularTransmissionDirection(inout Surface surface, float iorRation)
{
    surface.l = refract(-surface.v, iorRation);
    cacheBackSurfaceDots(surface);

    return iorRation * iorRation; // omit nol
}

// diffuse brdf * nol
vec3 SampleDiffuseDirection(inout Surface surface, inout uint seed, out float sampleWeight)
{
    vec2 u = rand2(seed); 
    surface.l = cosineSampleHemisphere(u);
    cacheSurfaceDots(surface);

    sampleWeight = 1.0 / cosineHemispherePdf(surface.nol);

    vec3 brdf_d = DisneyDiffuse(surface.nol, surface.nov, surface.loh, surface.a, surface.albedo);
	
	return brdf_d * surface.nol;
}

// returns mirror brdf * nol
float SampleSpecularReflectionDirection(inout Surface surface)
{
    surface.l = reflect(-surface.v);
    cacheSurfaceDots(surface);

    return BlinnPhongSpecular(surface.noh, surface.a);
}

// returns microfacet brdf * nol
float ImportanceSampleReflectionDirection(inout Surface surface, inout uint seed, out float sampleWeight)
{
    if(surface.a < SPEC_THRESHOLD){
        sampleWeight = 1.0;
        return SampleSpecularReflectionDirection(surface);
    }

    vec2 u = rand2(seed);
    vec3 H = importanceSampleGGX(u, surface.a);

    surface.l =  reflect(-surface.v, H);
    cacheSurfaceDots(surface);

	float pdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
    pdf = max(pdf, BIAS); // WIP: if small... notify to stop

    sampleWeight = 1 / pdf;

    return microfacetBrdfNoL(surface); 
}

// returns microfacet brdf * nol
float SampleReflectionDirection(inout Surface surface, inout uint seed, out float sampleWeight)
{
    if(surface.a < SPEC_THRESHOLD){
        sampleWeight = 1.0;
        return SampleSpecularReflectionDirection(surface);
    }

    vec2 u = rand2(seed); 
    surface.l = cosineSampleHemisphere(u);
    cacheSurfaceDots(surface);

    sampleWeight = 1.0 / cosineHemispherePdf(surface.nol);

    return microfacetBrdfNoL(surface); 
}

#endif
