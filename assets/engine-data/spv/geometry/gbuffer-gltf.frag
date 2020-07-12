#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

// out

layout (location = 0) out vec4 gNormal;
// rgb: base color, a: opacity
layout (location = 1) out vec4 gBaseColor;
// r: metallic, g: roughness, b: reflectance, a: occlusion
layout (location = 2) out vec4 gSurface;
layout (location = 3) out vec4 gEmissive;

// in

layout(location=0) in Data
{ 
	vec2 uv;
	mat3 TBN;
};

// uniforms

layout(set = 0, binding = 0) uniform UBO_Material {
	// factors
    vec4 baseColorFactor;
	vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// alpha mask
	float alphaCutoff;
	int mask;
} mat;

layout(set = 0, binding = 1) uniform sampler2D baseColorSampler;
layout(set = 0, binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(set = 0, binding = 3) uniform sampler2D occlusionSampler;
layout(set = 0, binding = 4) uniform sampler2D normalSampler;
layout(set = 0, binding = 5) uniform sampler2D emissiveSampler;

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
	vec3 albedo = sampledBaseColor.rgb * mat.baseColorFactor.rgb;
	float metallic = sampledMetallicRoughness.b * mat.metallicFactor;
	float roughness = sampledMetallicRoughness.g * mat.roughnessFactor;
	vec3 emissive = sampledEmissive.rgb * mat.emissiveFactor.rgb;
	float occlusion =  1 - mat.occlusionStrength * (1 - sampledOcclusion.r);
	vec3 normal = normalize((sampledNormal.rgb* 2.0 - 1.0) * vec3(mat.normalScale, mat.normalScale, 1.0));
	// opacity set from above

    // normal (with normal mapping)
    gNormal = vec4(normalize(TBN * normal.rgb), 1.f);
	
    gBaseColor = vec4(albedo, opacity);
	
	gSurface = vec4(metallic, roughness, 0.5, occlusion);
	
	gEmissive = vec4(emissive, 1.f);
}                                                                                        

