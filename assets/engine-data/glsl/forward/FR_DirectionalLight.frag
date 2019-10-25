#version 460 core

#include "/engine-data/glsl/include/random.h"
#include "/engine-data/glsl/include/microfacet_bsdf.h"
#include "/engine-data/glsl/include/poisson.h"

out vec4 out_color;
  
in Data
{ 
	vec3 wcs_fragPos;

	vec4 shadowCoord;
	
	vec2 textCoord[2];
	
	mat3 TBN;
} dataIn;

uniform struct DirectionalLight
{
	vec3 wcs_dir;

	vec3 color;
	float intensity;
	
	mat4 mvpBiased; // transforms to [0,1] in light space
	
	bool castsShadow;

	int samples;
	float maxShadowBias;
	sampler2DShadow shadowMap;
} directionalLight;

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

uniform vec3 wcs_viewPos;

float ShadowCalculation(float cosTheta)
{	
	if(!directionalLight.castsShadow)
		return 0.0;

	// texture(shadowMap, shadowCoord.xy).z is the distance between the light and the nearest occluder
	// shadowCoord.z is the distance between the light and the current fragment
	
	// cure shadow acne
	float bias = directionalLight.maxShadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, directionalLight.maxShadowBias);

	float currentDepth = dataIn.shadowCoord.z;
	vec2 shadowCoord = dataIn.shadowCoord.xy;

	// if in front or behind light's frustum return shadow
	if(currentDepth < 0.005
	|| currentDepth > 1.0)
		return 1.0;


	float shadow = 0;
	
	// Stratified Poisson Sampling
	for (int i = 0; i < directionalLight.samples; ++i)
	{
		int index = int(16.0*random(vec4(dataIn.wcs_fragPos,i)))%16;
		
		// Hardware implemented PCF on sample
		shadow += (1.0-texture(directionalLight.shadowMap, 
		vec3(shadowCoord + poissonDisk[index]/1400.0,  (currentDepth-bias))));
	}
		
    return shadow / directionalLight.samples;
}  

void ProcessUniformMaterial(out vec3 albedo, out float opacity, out float metallic, out float roughness, out vec3 normal)
{
	// sample material textures
	vec4 sampledBaseColor = texture(material.baseColorSampler, dataIn.textCoord[material.baseColorTexcoordIndex]);
	
	opacity = sampledBaseColor.a * material.baseColorFactor.a;
	// mask mode and cutoff
	if(material.mask && opacity < material.alphaCutoff)
		discard;
	
	vec4 sampledMetallicRoughness = texture(material.metallicRoughnessSampler, dataIn.textCoord[material.metallicRoughnessTexcoordIndex]);
	vec4 sampledNormal = texture(material.normalSampler, dataIn.textCoord[material.normalTexcoordIndex]);
	
	// final material values
	albedo = sampledBaseColor.rgb * material.baseColorFactor.rgb;
	metallic = sampledMetallicRoughness.b * material.metallicFactor;
	roughness = sampledMetallicRoughness.g * material.roughnessFactor;
	normal = normalize((sampledNormal.rgb * 2.0 - 1.0) * vec3(material.normalScale, material.normalScale, 1.0));
	// opacity set from above
}

void main()
{
	vec3 albedo;
	float opacity;
	float metallic;
	float roughness;
	vec3 normal;
	
	// material
	ProcessUniformMaterial(albedo, opacity, metallic, roughness, normal);

	// TODO: implement tangent space lighting correctly
	// world space vectors
	vec3 N =  normalize(dataIn.TBN * normal);
	vec3 V = normalize(wcs_viewPos - dataIn.wcs_fragPos);
	vec3 L = normalize(-directionalLight.wcs_dir); 

	float shadow = ShadowCalculation(max(dot(N, L), 0.0)); 
	vec3 Li = (1.0 - shadow) * directionalLight.color * directionalLight.intensity; 
	
	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, albedo, metallic, roughness) * Li * max(dot(N, L), 0.0);

	out_color = vec4(Lo, opacity);
}