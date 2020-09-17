#ifndef fragment_glsl
#define fragment_glsl

struct Fragment
{
    vec3 position;
    vec3 normal;
    vec3 diffuseColor;
    vec3 f0;
    vec3 emissive;
    float a;
    float opacity;
    float occlusion;
    float depth;
};

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

Fragment getFragmentFromGBuffer(
    float depth, 
    mat4 viewProjInv,
    sampler2D normalsSampler, 
    sampler2D diffuseOpacitySampler,
    sampler2D f0RoughnessSampler,
    sampler2D emissiveOcclusionSampler,
    vec2 uv)
{
    Fragment frag;

    frag.position = reconstructWorldPosition(depth, uv, viewProjInv);
    frag.normal = texture(normalsSampler, uv).rgb;
    // rgb: albedo a: opacity
    vec4 diffuseOpacity = texture(diffuseOpacitySampler, uv);
    // rgb: f0, a: a: roughness^
    vec4 f0Roughness = texture(f0RoughnessSampler, uv);
    // rgb: emissive, a: occlusion
    vec4 emissiveOcclusion = texture(emissiveOcclusionSampler, uv);

    // remapping
    frag.diffuseColor = diffuseOpacity.rgb;
    frag.opacity = diffuseOpacity.a;

	frag.f0 = f0Roughness.rgb;
	frag.a = f0Roughness.a;

    frag.emissive = emissiveOcclusion.rgb;
    frag.occlusion = emissiveOcclusion.a;

    frag.depth = depth;

    return frag;
}

#endif