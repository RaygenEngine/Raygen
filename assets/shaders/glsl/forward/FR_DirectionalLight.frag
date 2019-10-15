#version 460 core

out vec4 out_color;
  
in Data
{ 
	vec3 tcs_fragPos;
	vec3 tcs_viewPos;
	
	vec3 tcs_lightDir;
	
	vec4 shadowCoord;
	
	vec2 textCoord[2];
} dataIn;

uniform struct DirectionalLight
{
	vec3 wcs_dir;

	vec3 color;
	float intensity;
	
	mat4 mvpBiased; // transforms to [0,1] in light space

	int samples;
	float maxShadowBias;
	sampler2DShadow shadowMap;
} directionalLight;

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

vec2 poissonDisk[16] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760),
	vec2(-0.91588581, 0.45771432),
	vec2(-0.81544232, -0.87912464),
	vec2(-0.38277543, 0.27676845),
	vec2(0.97484398, 0.75648379),
	vec2(0.44323325, -0.97511554),
	vec2(0.53742981, -0.47373420),
	vec2(-0.26496911, -0.41893023),
	vec2(0.79197514, 0.19090188),
	vec2(-0.24188840, 0.99706507),
	vec2(-0.81409955, 0.91437590),
	vec2(0.19984126, 0.78641367),
	vec2(0.14383161, -0.14100790)
	);
	
float random(vec4 seed4)
{
	float dot = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot) * 43758.5453);
}

float ShadowCalculation(float cosTheta)
{	
	// texture(shadowMap, shadowCoord.xy).z is the distance between the light and the nearest occluder
	// shadowCoord.z is the distance between the light and the current fragment
	
	// cure shadow acne
	float bias = directionalLight.maxShadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, directionalLight.maxShadowBias);

	float currentDepth = dataIn.shadowCoord.z;
	vec2 shadowCoord = dataIn.shadowCoord.xy;

	// if behind shadow map just return shadow
	if(currentDepth < 0.005)
		return 1.0;


	float shadow = 0;
	
	// Stratified Poisson Sampling
	for (int i = 0; i < directionalLight.samples; ++i)
	{
		int index = int(16.0*random(vec4(dataIn.tcs_fragPos,i)))%16;
		
		// Hardware implemented PCF on sample
		shadow += (1.0-texture(directionalLight.shadowMap, 
		vec3(shadowCoord + poissonDisk[index]/1400.0,  (currentDepth-bias))));
	}
		
    return shadow / directionalLight.samples;
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
	
	// NOTE: directional: same direction and no attenuation
	vec3 L = -dataIn.tcs_lightDir;

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
	
	float shadow = ShadowCalculation(max(dot(N, L), 0.0)); 
	vec3 radiance = (1.0 - shadow) * directionalLight.color * directionalLight.intensity; 
	
	vec3 Lo = (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, material.occlusionStrength);
	
	// TODO:
	//color = powVec3(color, 1.0f / 2.2f);
	
	out_color = vec4(vec3(color), opacity);
}