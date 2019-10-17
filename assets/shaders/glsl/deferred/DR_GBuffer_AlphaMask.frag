#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoOpacity;
// r: metallic, g: roughness, b: occlusion, a: occlusion strength
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;
  
in Data
{ 
	vec3 wcs_fragPos; 

	vec2 textCoord[2];
	
	mat3 TBN;
} dataIn;

uniform struct Material
{
	// factors
	vec4 baseColorFactor;
	vec3 emissiveFactor;
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
	
	// samplers
	sampler2D baseColorSampler;
	sampler2D metallicRoughnessSampler;
	sampler2D emissiveSampler;
	sampler2D normalSampler;
	sampler2D occlusionSampler;
	
	// alpha mask
	float alphaCutoff;
	bool mask;

} material;

void ProcessUniformMaterial(out vec3 albedo, out float opacity, out float metallic, out float roughness,
							out vec3 emissive, out float occlusion, out vec3 normal)
{
	// sample material textures
	vec4 sampledBaseColor = texture(material.baseColorSampler, dataIn.textCoord[material.baseColorTexcoordIndex]);
	
	opacity = sampledBaseColor.a * material.baseColorFactor.a;
	// mask mode and cutoff
	if(material.mask && opacity < material.alphaCutoff)
		discard;
	
	vec4 sampledMetallicRoughness = texture(material.metallicRoughnessSampler, dataIn.textCoord[material.metallicRoughnessTexcoordIndex]); 
	vec4 sampledEmissive = texture(material.emissiveSampler, dataIn.textCoord[material.emissiveTexcoordIndex]);
	vec4 sampledNormal = texture(material.normalSampler, dataIn.textCoord[material.normalTexcoordIndex]);
	vec4 sampledOcclusion = texture(material.occlusionSampler, dataIn.textCoord[material.occlusionTexcoordIndex]);
	
	// final material values
	albedo = sampledBaseColor.rgb * material.baseColorFactor.rgb;
	metallic = sampledMetallicRoughness.b * material.metallicFactor;
	roughness = sampledMetallicRoughness.g * material.roughnessFactor;
	emissive = sampledEmissive.rgb * material.emissiveFactor;
	occlusion = sampledOcclusion.r;
	normal = normalize((sampledNormal.rgb * 2.0 - 1.0) * vec3(material.normalScale, material.normalScale, 1.0));
	// opacity set from above
}

void main()
{
	vec3 albedo;
	float opacity;
	float metallic;
	float roughness;
	vec3 emissive;
	float occlusion;
	vec3 normal;
	
	// material
	ProcessUniformMaterial(albedo, opacity, metallic, roughness, emissive, occlusion, normal);

    // position
    gPosition = dataIn.wcs_fragPos;
	
    // normal (with normal mapping)
    gNormal = normalize(dataIn.TBN * normal);
	
    // albedo opacity
    gAlbedoOpacity = vec4(albedo, opacity);
	
	// spec params
	gSpecular = vec4(metallic, roughness, occlusion, material.occlusionStrength);
	
	// emissive
	gEmissive = vec4(emissive, 1.f);
}