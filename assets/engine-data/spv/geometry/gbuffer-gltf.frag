#version 460
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

// out

layout (location = 0) out vec4 gSNormal;

layout (location = 1) out vec4 gGNormal;

// rgb: albedo, a: opacity
layout (location = 2) out vec4 gAlbedo;
// r: f0, a: a (roughness^2)
layout (location = 3) out vec4 gSpecularColor;
// rgb: emissive, a: occlusion
layout (location = 4) out vec4 gEmissive;

layout (location = 5) out vec4 gVelocity;
layout (location = 6) out vec4 gUVDrawIndex;

// in

layout(location=0) in Data
{ 
	vec2 uv;
	mat3 TBN;
	vec3 fragPos;
	vec4 clipPos;
	vec4 prevClipPos;
	float drawIndex;
};

// uniforms

layout(set = 1, binding = 0) uniform UBO_Material { GltfMaterial mat; };
layout(set = 1, binding = 1) uniform sampler2D baseColorSampler;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(set = 1, binding = 3) uniform sampler2D occlusionSampler;
layout(set = 1, binding = 4) uniform sampler2D normalSampler;
layout(set = 1, binding = 5) uniform sampler2D emissiveSampler;

void main() {
	// sample material textures
	vec4 sampledBaseColor = texture(baseColorSampler, uv);
	
	float opacity = sampledBaseColor.a * mat.baseColorFactor.a;
	// mask mode and cutoff
	if(mat.mask == 1 && opacity < mat.alphaCutoff)
		discard;
	
	vec4 sampledMetallicRoughness = texture(metallicRoughnessSampler, uv); 
	vec4 sampledEmissive = texture(emissiveSampler, uv);
	vec4 sampledNormal = texture(normalSampler, uv);
	vec4 sampledOcclusion = texture(occlusionSampler, uv);
	
	// final material values
	vec3 baseColor = sampledBaseColor.rgb * mat.baseColorFactor.rgb;
	float metallic = sampledMetallicRoughness.b * mat.metallicFactor;
	float roughness = sampledMetallicRoughness.g * mat.roughnessFactor;
	vec3 emissive = sampledEmissive.rgb * mat.emissiveFactor.rgb;
	float occlusion =  1.0 - mat.occlusionStrength * (1.0 - sampledOcclusion.r);
	vec3 normal = normalize((sampledNormal.rgb* 2.0 - 1.0) * vec3(mat.normalScale, mat.normalScale, 1.0));
	// opacity set from above

    // normal (with normal mapping)
    gSNormal = vec4(normalize(TBN * normal), 1.0);

	// geometric normal
	gGNormal = vec4(TBN[2], 1.0);
	
	// albedo = (1.0 - metallic) * baseColor;
	gAlbedo = vec4((1.0 - metallic) * baseColor, opacity);

	// CHECK: reflectance
	// f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + albedo * metallic;
	gSpecularColor = vec4(vec3(0.16 * 0.5 * 0.5 * (1.0 - metallic)) + baseColor * metallic, roughness * roughness);
	
	gEmissive = vec4(emissive, occlusion);
	
	vec2 a = (clipPos.xy / clipPos.w) * 0.5 + 0.5;
    vec2 b = (prevClipPos.xy / prevClipPos.w) * 0.5 + 0.5;

	float expectedDepth = (prevClipPos.z / prevClipPos.w);

	gVelocity = vec4(b - a, expectedDepth, 1.f);
	gUVDrawIndex = vec4(uv, drawIndex, 1.f);
}                                                                                        
