#version 460 core

#include "/shaders/glsl/include/random.h"
#include "/shaders/glsl/include/microfacet_bsdf.h"
#include "/shaders/glsl/include/poisson.h"

out vec4 out_color;
  
in Data
{ 
	vec3 tcs_fragPos;
	vec3 tcs_viewPos;
	
	vec3 tcs_lightPos;
	vec3 tcs_lightDir;
	
	vec4 shadowCoord;
	
	vec2 textCoord[2];
} dataIn;


uniform struct SpotLight
{
	vec3 wcs_pos;
	vec3 wcs_dir;
	
	float outerCutOff;
	float innerCutOff;
	
	vec3 color;
	float intensity;
	
	int attenCoef;

	mat4 mvpBiased; // transforms to [0,1] in light space
	
	int samples;
	float maxShadowBias;
	sampler2DShadow shadowMap;
} spotLight;


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

float ShadowCalculation(float cosTheta)
{	
	// texture(shadowMap, shadowCoord.xy).z is the distance between the light and the nearest occluder
	// shadowCoord.z is the distance between the light and the current fragment
	
	// cure shadow acne
	float bias = spotLight.maxShadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, spotLight.maxShadowBias);

	float currentDepth = (dataIn.shadowCoord.z - bias);

	// if behind shadow map just return shadow
	if(currentDepth / dataIn.shadowCoord.w < 0.005)
		return 1.0;
		
	float shadow = 0;
	
	// Stratified Poisson Sampling
	for (int i = 0; i < spotLight.samples; ++i)
	{
		int index = int(16.0*random(vec4(dataIn.tcs_fragPos,i)))%16;
		
		vec2 shadowCoord = dataIn.shadowCoord.xy + poissonDisk[index]/100.0;
		vec4 P = vec4(shadowCoord.xy, currentDepth, dataIn.shadowCoord.w);
		
		// Hardware implemented PCF on sample
		shadow += (1.0-textureProj(spotLight.shadowMap, P, 0.f));
	}
		
    return shadow / spotLight.samples;
}  

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

	// tangent space vectors
	vec3 N = normal;
	vec3 V = normalize(dataIn.tcs_viewPos - dataIn.tcs_fragPos);
	vec3 L = normalize(dataIn.tcs_lightPos - dataIn.tcs_fragPos); 
	
	// attenuation
	float distance = length(dataIn.tcs_lightPos - dataIn.tcs_fragPos);
	float attenuation = 1.0 / pow(distance, spotLight.attenCoef);
	
    // spot effect (soft edges)
	float theta = dot(L, normalize(-dataIn.tcs_lightDir));
    float epsilon = (spotLight.innerCutOff - spotLight.outerCutOff);
    float spotEffect = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
	 
	float shadow = ShadowCalculation(max(dot(N, L), 0.0)); 
	vec3 Li = ((1.0 - shadow) * spotLight.color * spotLight.intensity * attenuation * spotEffect); 

	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, albedo, metallic, roughness) * Li * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, material.occlusionStrength);
	
	out_color = vec4(color, opacity);
}