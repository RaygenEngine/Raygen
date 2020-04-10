#ifndef fragment_h
#define fragment_h

struct Fragment
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
    float opacity;
    vec3 emissive;
    float metallic;
    float roughness;
    float depth;
};

Fragment GetFragmentFromGBuffer(
    in sampler2D positionsSampler, 
    in sampler2D normalsSampler, 
    in sampler2D albedoOpacitySampler,
    in sampler2D specularSampler,
    in sampler2D emissiveSampler,
    in sampler2D depthSampler,
    vec2 uv)
{
    Fragment fragment;

    fragment.position = texture(positionsSampler, uv).rgb;
    fragment.normal = texture(normalsSampler, uv).rgb;
    vec4 albedoOpacity = texture(albedoOpacitySampler, uv);
    fragment.albedo = albedoOpacity.rgb;
    fragment.opacity = albedoOpacity.a;
    fragment.emissive = texture(emissiveSampler, uv).rgb;
    vec4 metallicRoughnessOcclusionOcclusionStrength = texture(specularSampler, uv);
    fragment.metallic = metallicRoughnessOcclusionOcclusionStrength.r;
    fragment.roughness = metallicRoughnessOcclusionOcclusionStrength.g;
    fragment.depth = texture(depthSampler, uv).r;

    return fragment;
}

#endif