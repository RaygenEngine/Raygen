#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
// rgb: albedo, a: opacity
layout (location = 2) out vec4 gAlbedoOpacity;
// r: metallic, g: roughness, b: occlusion, a: occlusion strength
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;

// in

layout(location=0) in Data
{ 
	vec3 fragPos; 
	vec2 uv[2];
	mat3 TBN;
};

// uniforms

layout(binding = 1) uniform UBO_Material {
	// factors
    vec4 baseColorFactor;
	vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// text coord indices
	int baseColorTexcoordIndex;
	int metallicRoughnessTexcoordIndex;
	int emissiveTexcoordIndex;
	int normalTexcoordIndex;
	int occlusionTexcoordIndex;

	// alpha mask
	float alphaCutoff;
	int mask;
} material;

layout(binding = 2) uniform sampler2D baseColorSampler;
layout(binding = 3) uniform sampler2D metallicRoughnessSampler;
layout(binding = 4) uniform sampler2D occlusionSampler;
layout(binding = 5) uniform sampler2D normalSampler;
layout(binding = 6) uniform sampler2D emissiveSampler;

void main() {
	// sample material textures
	vec4 sampledBaseColor = texture(baseColorSampler, uv[material.baseColorTexcoordIndex]);
	
	float opacity = sampledBaseColor.a * material.baseColorFactor.a;
	// mask mode and cutoff
	if(material.mask == 1 && opacity < material.alphaCutoff)
		discard;
	
	vec4 sampledMetallicRoughness = texture(metallicRoughnessSampler, uv[material.metallicRoughnessTexcoordIndex]); 
	vec4 sampledEmissive = texture(emissiveSampler, uv[material.emissiveTexcoordIndex]);
	vec4 sampledNormal = texture(normalSampler, uv[material.normalTexcoordIndex]);
	vec4 sampledOcclusion = texture(occlusionSampler, uv[material.occlusionTexcoordIndex]);
	
	// final material values
	vec3 albedo = sampledBaseColor.rgb * material.baseColorFactor.rgb;
	float metallic = sampledMetallicRoughness.b * material.metallicFactor;
	float roughness = sampledMetallicRoughness.g * material.roughnessFactor;
	vec3 emissive = sampledEmissive.rgb * material.emissiveFactor.rgb;
	float occlusion = sampledOcclusion.r;
	vec3 normal = normalize((sampledNormal.rgb * 2.0 - 1.0) * vec3(material.normalScale, material.normalScale, 1.0));
	// opacity set from above

    // position
    gPosition = fragPos;
	
    // normal (with normal mapping)
    gNormal = normalize(TBN * normal);
	
    // albedo opacity
    gAlbedoOpacity = vec4(albedo, opacity);
	
	// spec params
	gSpecular = vec4(metallic, roughness, occlusion, material.occlusionStrength);
	
	// emissive
	gEmissive = vec4(emissive, 1.f);
}