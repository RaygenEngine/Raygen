#ifndef fragment_h
#define fragment_h

struct Fragment
{
    vec3 position;
    vec3 normal;
    vec3 baseColor;
    float opacity;
    vec3 emissive;
    float metallic;
    float roughness;
    float reflectance;
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
    Fragment fragment;

    fragment.position = reconstructWorldPosition(depth, uv, viewProjInv);
    fragment.normal = texture(normalsSampler, uv).rgb;
    vec4 albedoOpacity = texture(albedoOpacitySampler, uv);
    fragment.baseColor = albedoOpacity.rgb;
    fragment.opacity = albedoOpacity.a;
    fragment.emissive = texture(emissiveSampler, uv).rgb;
    vec4 metallicRoughnessReflectanceOcclusion = texture(surfaceSampler, uv);
    fragment.metallic = metallicRoughnessReflectanceOcclusion.r;
    fragment.roughness = metallicRoughnessReflectanceOcclusion.g;
    fragment.reflectance = metallicRoughnessReflectanceOcclusion.b;
    fragment.depth = depth;

    return fragment;
}

#endif