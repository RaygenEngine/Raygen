#ifndef surface_glsl
#define surface_glsl

#include "onb.glsl"

struct Surface {
    
    // surface
    Onb basis; // shading normal aligned hemisphere
    vec3 position;
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
};

void addShadingNormal(inout Surface surface, vec3 N)
{
    surface.basis = branchlessOnb(N);
}

void addGeometricNormal(inout Surface surface, vec3 Ng)
{
    surface.ng = toOnbSpace(surface.basis, Ng);
}

void addIncomingDir(inout Surface surface, vec3 V)
{
    surface.i = toOnbSpace(surface.basis, V);
}

void addOutgoingDir(inout Surface surface, vec3 L)
{
    surface.o = toOnbSpace(surface.basis, L);
    surface.h = normalize(surface.i + surface.o);
}

vec3 getOutgoingDir(Surface surface)
{
    return outOnbSpace(surface.basis, surface.o);
}

vec3 getIncomingDir(Surface surface)
{
    return outOnbSpace(surface.basis, surface.i);
}

void addInitialVectors(inout Surface surface, vec3 geometricNormal, vec3 shadingNormal, vec3 incomingDirection) 
{
    addShadingNormal(surface, shadingNormal); // the onb is built based on the shading normal
    addGeometricNormal(surface, geometricNormal); // the geometric normal is in reference to the shading normal to match other vectors

    addIncomingDir(surface, incomingDirection);
}

bool isOutgoingDirPassingThrough(Surface surface)
{
    // below actual surface normal -> passes through
    return dot(surface.ng, surface.o) < 0;
}

#include "surface-bsdf.glsl"

Surface surfaceFromGBuffer(
    Camera cam,
    float depth,
    sampler2D snormalsSampler,
    sampler2D gnormalsSampler,
    sampler2D albedoOpacitySampler,
    sampler2D f0RoughnessSampler,
    sampler2D emissiveOcclusionSampler,
    vec2 uv)
{
    Surface surface;

    surface.position = reconstructWorldPosition(depth, uv, cam.viewProjInv);

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

    // Geometric stuff
	float f0 = max(surface.f0);
	float sqrtf0 = sqrt(f0);
	float eta = -(f0 + 1 + 2 * sqrtf0) / (f0 - 1);

	// Backwards tracing:
	surface.eta_i = 1.0; // vacuum view ray
	surface.eta_o = eta; // material light ray

    vec3 N = texture(snormalsSampler, uv).rgb;
    vec3 Ng = texture(gnormalsSampler, uv).rgb;
    vec3 V = normalize(cam.position - surface.position);
	
	// from the inside of object - if not culled
	if(dot(V, Ng) < 0) { 
		Ng = -Ng;
		N = -N;
		
		surface.eta_o = 1.0; // vacuum light ray
		surface.eta_i = eta; // material view ray
	}

    addInitialVectors(surface, Ng, N, V);

    return surface;
}

Surface surfaceFromGBufferNoMaterial(
    Camera cam,
    float depth,
    sampler2D snormalsSampler,
    sampler2D gnormalsSampler,
    vec2 uv)
{
    Surface surface;

    surface.position = reconstructWorldPosition(depth, uv, cam.viewProjInv);

    vec3 N = texture(snormalsSampler, uv).rgb;
    vec3 Ng = texture(gnormalsSampler, uv).rgb;
    vec3 V = normalize(cam.position - surface.position);

    // from the inside of object - if not culled
    if (dot(V, Ng) < 0) {
        Ng = -Ng;
        N = -N;
    }

    addInitialVectors(surface, Ng, N, V);

    return surface;
}

#ifndef RAY
Surface surfaceFromGBuffer(
    Camera cam,
    float depth,
    subpassInput snormalsInput,
    subpassInput gnormalsInput,
    subpassInput albedoOpacityInput,
    subpassInput f0RoughnessInput,
    subpassInput emissiveOcclusionInput,
    vec2 uv)
{
    Surface surface;

    surface.position = reconstructWorldPosition(depth, uv, cam.viewProjInv);

    // rgb: albedo a: opacity
    vec4 albedoOpacity = subpassLoad(albedoOpacityInput);
    // rgb: f0, a: roughness^2
    vec4 f0Roughness = subpassLoad(f0RoughnessInput);
    // rgb: emissive, a: occlusion
    vec4 emissiveOcclusion = subpassLoad(emissiveOcclusionInput);

    surface.albedo = albedoOpacity.rgb;
    surface.opacity = albedoOpacity.a;

	surface.f0 = f0Roughness.rgb;
	surface.a = f0Roughness.a;

    surface.emissive = emissiveOcclusion.rgb;
    surface.occlusion = emissiveOcclusion.a;

        // Geometric stuff
	float f0 = max(surface.f0);
	float sqrtf0 = sqrt(f0);
	float eta = -(f0 + 1 + 2 * sqrtf0) / (f0 - 1);

	// Backwards tracing:
	surface.eta_i = 1.0; // vacuum view ray
	surface.eta_o = eta; // material light ray

    vec3 N = subpassLoad(snormalsInput).rgb;
    vec3 Ng = subpassLoad(gnormalsInput).rgb;
    vec3 V = normalize(cam.position - surface.position);
	
	// from the inside of object - if not culled
	if(dot(V, Ng) < 0) { 
		Ng = -Ng;
		N = -N;
		
		surface.eta_o = 1.0; // vacuum light ray
		surface.eta_i = eta; // material view ray
	}

    addInitialVectors(surface, Ng, N, V);

    return surface;
}
#endif

#endif
