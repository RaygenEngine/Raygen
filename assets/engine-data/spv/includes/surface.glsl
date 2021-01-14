#ifndef surface_glsl
#define surface_glsl

#include "bsdfs.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"

struct Surface {
    
    // surface
    Onb basis;

    vec3 ng;
    
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
    float eta; // of interaction, depends on current medium
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

bool isIncidentLightDirAboveSurfaceGeometry(Surface surface)
{
    return dot(surface.ng, surface.l) > 0; // WIP: test normals
}

void cacheSurfaceDots(inout Surface surface)
{
    vec3 l = surface.l;
    // if l and v are not in the same hemisphere 
    if(!sameHemisphere(surface.v, surface.l)) {
        l.z *= -1; // bring l in the same for correct dot calculation
    }

    surface.h = normalize(surface.v + l); 
    surface.noh = absNdot(surface.h);
    surface.loh = absdot(l, surface.h);
    surface.nol = absNdot(l);
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

// SMATH: check math of microfacet functions
float microfacetBrdf(Surface surface) 
{
    float NDF = D_GGX(surface.noh, surface.a);
    float G = G_SmithSchlickGGX(surface.nol, surface.nov, surface.a);

    float numerator = NDF * G;
    float denominator = 4.0 * surface.nov * surface.nol;

    return numerator / max(denominator, 0.001); 
}

float microfacetBtdf(Surface surface) 
{
    float NDF = D_GGX(surface.noh, surface.a);
    float G = G_SmithSchlickGGX(surface.nol, surface.nov, surface.a);

    float numerator = NDF * G * surface.loh * surface.loh;
    float denominator = (surface.loh + surface.eta * surface.loh); 
    denominator *= denominator;
    denominator *= surface.nov * surface.nol;

    return numerator / max(denominator, 0.001); 
}

// SMATH: check opacity stuff, and specular
vec3 explicitBrdf(inout Surface surface, vec3 L)
{
    addIncomingLightDirection(surface, L);

    vec3 ks = F_Schlick(surface.loh, surface.f0);
    vec3 kt = 1.0 - ks;
    vec3 kd = kt * surface.opacity;

    vec3 brdf_d = LambertianDiffuse(surface.albedo);
 
    float brdf_r =surface.a >= SPEC_THRESHOLD ? microfacetBrdf(surface) : 0.f;

    return kd * brdf_d + ks * brdf_r;
}

#endif
