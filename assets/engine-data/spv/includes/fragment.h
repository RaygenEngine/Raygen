#ifndef fragment_h
#define fragment_h

// GBuffer
layout(set = 0, binding = 0) uniform sampler2D g_DepthSampler;
layout(set = 0, binding = 1) uniform sampler2D g_NormalSampler;
layout(set = 0, binding = 2) uniform sampler2D g_ColorSampler;
layout(set = 0, binding = 3) uniform sampler2D g_MRROSampler; // Metallic Roughness Reflectance Occlusion
layout(set = 0, binding = 4) uniform sampler2D g_EmissiveSampler;
											 
// Raster Direct                             
layout(set = 0, binding = 5) uniform sampler2D rasterDirectSampler;
											 
// RayTracing                                
layout(set = 0, binding = 6) uniform sampler2D rtIndirectSampler;
											 
// Blend Rast + Ray                          
layout(set = 0, binding = 7) uniform sampler2D sceneColorSampler;

struct Fragment
{
    vec3 position;
    vec3 normal;
    vec3 diffuseColor;
    vec3 f0;
    vec3 emissive;
    float a;
    float opacity;
    float depth;
};

vec3 reconstructWorldPosition(float depth, vec2 uv, in mat4 viewProjInv)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 worldPos = viewProjInv * clipPos;

	return worldPos.xyz / worldPos.w; // return world space pos xyz
}

Fragment getFragmentFromGBuffer(
    float depth, 
    in mat4 viewProjInv,
    in sampler2D normalsSampler, 
    in sampler2D albedoOpacitySampler,
    in sampler2D surfaceSampler,
    in sampler2D emissiveSampler,
    vec2 uv)
{
    Fragment frag;

    frag.position = reconstructWorldPosition(depth, uv, viewProjInv);
    frag.normal = texture(normalsSampler, uv).rgb;
    // rgb: albedo a: opacity
    vec4 albedoOpacity = texture(albedoOpacitySampler, uv);
    // r: metallic g: roughness b: reflectance a: occlusion
    vec4 mrro = texture(surfaceSampler, uv);

    // remapping

	// diffuseColor = (1.0 - metallic) * albedo;
    frag.diffuseColor = (1.0 - mrro.r) * albedoOpacity.rgb;
    // f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + albedo * metallic;
	frag.f0 = vec3(0.16 * mrro.b * mrro.b * (1.0 - mrro.r)) + albedoOpacity.rgb * mrro.r;
    // a = roughness roughness;
	frag.a = mrro.g * mrro.g;

    frag.opacity = albedoOpacity.a;
    frag.emissive = texture(emissiveSampler, uv).rgb;
    frag.depth = depth;

    return frag;
}

#endif