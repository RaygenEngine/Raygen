#version 460 core

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

	int samples;
	float maxShadowBias;
	samplerCube shadowCubemap;
} punctualLight;

// dont instantiate this (non-uniformly)
struct SamplerWithTexcoordIndex
{
	int index;
	sampler2D sampler;
};

uniform struct Material
{
	vec4 baseColorFactor;
	vec3 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;
	
	SamplerWithTexcoordIndex baseColorSampler;
	SamplerWithTexcoordIndex metallicRoughnessSampler;
	SamplerWithTexcoordIndex emissiveSampler;
	SamplerWithTexcoordIndex normalSampler;
	SamplerWithTexcoordIndex occlusionSampler;
	
	float alphaCutoff;
	bool mask;

} material;

#define PI 3.14159265358979323846f

float ShadowCalculation(float cosTheta)
{	
	// get vector between fragment position and light position
    vec3 fragToLight = dataIn.wcs_fragPos - punctualLight.wcs_pos;
	
	float currentDepth = length(fragToLight);
	
	float shadow  = 0.0;
	float bias = punctualLight.maxShadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, punctualLight.maxShadowBias);
	float samples = punctualLight.samples;
	float offset  = 0.1;
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

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec4 SampleMaterialTexture(SamplerWithTexcoordIndex si)
{
	return texture(si.sampler, dataIn.textCoord[si.index]);
}

void ProcessUniformMaterial(out vec3 albedo, out float opacity, out float metallic, out float roughness,
							out vec3 emissive, out float occlusion, out vec3 normal)
{
	// sample material textures
	vec4 sampledBaseColor = SampleMaterialTexture(material.baseColorSampler);
	
	opacity = sampledBaseColor.a * material.baseColorFactor.a;
	// mask mode and cutoff
	if(material.mask && opacity < material.alphaCutoff)
		discard;
	
	vec4 sampledMetallicRoughness = SampleMaterialTexture(material.metallicRoughnessSampler);
	vec4 sampledEmissive = SampleMaterialTexture(material.emissiveSampler);
	vec4 sampledNormal = SampleMaterialTexture(material.normalSampler);
	vec4 sampledOcclusion = SampleMaterialTexture(material.occlusionSampler);
	
	// final material values
	albedo = sampledBaseColor.rgb * material.baseColorFactor.rgb;
	metallic = sampledMetallicRoughness.b * material.metallicFactor;
	roughness = sampledMetallicRoughness.g * material.roughnessFactor;
	emissive = sampledEmissive.rgb * material.emissiveFactor;
	occlusion = sampledOcclusion.r;
	normal = normalize((sampledNormal.rgb * 2.0 - 1.0) * vec3(material.normalScale, material.normalScale, 1.0));
	// opacity set from above
}

vec3 powVec3 (vec3 v, float f)
{
	vec3 res;
	res.x = pow(v.x, f);
	res.y = pow(v.y, f);
	res.z = pow(v.z, f);
	return res;
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
	
	ProcessUniformMaterial(albedo, opacity, metallic, roughness, emissive, occlusion, normal);

	// NOTE: all calculations should be in tangent space
	vec3 N = normal;
	
	vec3 V = normalize(dataIn.tcs_viewPos - dataIn.tcs_fragPos);
	
	// loop lights
	vec3 L = normalize(dataIn.tcs_lightPos - dataIn.tcs_fragPos);  
	vec3 H = normalize(V + L);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
	
	float NDF = DistributionGGX(N, H, roughness);       
	float G = GeometrySmith(N, V, L, roughness); 
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001); 

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;
	
	// attenuation
	float distance = length(dataIn.tcs_lightPos - dataIn.tcs_fragPos);
	float attenuation = 1.0 / pow(distance, punctualLight.attenCoef);
	
	float shadow = ShadowCalculation(max(dot(N, L), 0.0)); 
	vec3 radiance = ((1.0 - shadow) * punctualLight.color * punctualLight.intensity * attenuation); 

	vec3 Lo = (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, material.occlusionStrength);
	
	// TODO:
	//color = powVec3(color, 1.0f / 2.2f);
	
	out_color = vec4(color, opacity);
}