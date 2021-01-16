#ifndef surface_glsl
#define surface_glsl

#include "onb.glsl"

struct Surface {
    
    // surface
    Onb basis; // shading normal aligned hemisphere

    vec3 ng; // geometric normal
    
    // we use i and surface interface to sample o directions
    // h is calculated from i and o according to interface (refraction, reflection, etc)
    vec3 i; // incoming
    vec3 o; // outgoing
    vec3 h; // microsurface normal

    // material
    vec3 albedo;
    vec3 f0;
    vec3 emissive;
    float a;
    float eta_i; // eta of medium of incoming ray
    float eta_o; // eta of medium of outgoing ray
    float opacity;
    float occlusion;

    // surface
    vec3 position; 
    float depth;
    vec2 uv;
};

vec3 getOutgoingDir(Surface surface)
{
    return outOnbSpace(surface.basis, surface.o);
}

vec3 getIncomingDir(Surface surface)
{
    return outOnbSpace(surface.basis, surface.i);
}

void addOutgoingDir(inout Surface surface, vec3 L)
{
    surface.o = toOnbSpace(surface.basis, L);
    surface.h = normalize(surface.i + surface.o);
}

bool isOutgoingDirPassingThrough(Surface surface)
{
    // not in the same side of ng -> passes through
    return dot(surface.ng, surface.o) < 0;
}

#include "surface-bsdf.glsl"

// WIP: tidy those
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
    surface.i = normalize(toOnbSpace(surface.basis, V));
    //surface.nov = max(Ndot(surface.v), BIAS);

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
    surface.i = normalize(toOnbSpace(surface.basis, V));
    //surface.nov = max(Ndot(surface.v), BIAS);

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

#endif
