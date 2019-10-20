#version 460 core

#include "/engine-data/glsl/include/microfacet_bsdf.h"

out vec4 out_color;
  
in Data
{ 
	vec3 wcs_fragPos;

	vec3 tcs_fragPos;
	vec3 tcs_viewPos;
	
	vec3 tcs_lightPos;

	vec2 textCoord[2];
} dataIn;

uniform struct PunctualLight
{
	vec3 wcs_pos;
	
	vec3 color;
	float intensity;
	
	float far;

	int attenCoef;
	
	bool castsShadow;

	int samples;
	float maxShadowBias;
	samplerCube shadowCubemap;
} punctualLight;

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
	if(!punctualLight.castsShadow)
		return 0.0;


	// get vector between fragment position and light position
    vec3 fragToLight = dataIn.wcs_fragPos - punctualLight.wcs_pos;
	
	float currentDepth = length(fragToLight);
	
	// 3d pcf
	float shadow  = 0.0;
	float bias = punctualLight.maxShadowBias;
	float samples = punctualLight.samples;
	float offset  = 0.05;
	for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	{
		for(float y = -offset; y < offset; y += offset / (samples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (samples * 0.5))
			{
				float closestDepth = texture(punctualLight.shadowCubemap, fragToLight + vec3(x, y, z)).r; 
				closestDepth *= punctualLight.far;   // Undo mapping [0;1]
				if(currentDepth - bias > closestDepth)
					shadow += 1.0;
			}
		}
	}
	
	shadow /= (samples * samples * samples);	

    return shadow;
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
	float attenuation = 1.0 / pow(distance, punctualLight.attenCoef);
	
	float shadow = ShadowCalculation(max(dot(N, L), 0.0)); 
	vec3 Li = ((1.0 - shadow) * punctualLight.color * punctualLight.intensity * attenuation); 

	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, albedo, metallic, roughness) * Li * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, material.occlusionStrength);
		
	out_color = vec4(color, opacity);
}