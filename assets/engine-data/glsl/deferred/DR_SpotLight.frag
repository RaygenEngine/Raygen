#version 460 core

#include "/engine-data/glsl/include/random.h"
#include "/engine-data/glsl/include/microfacet_bsdf.h"
#include "/engine-data/glsl/include/poisson.h"

out vec4 out_color;

uniform vec3 wcs_viewPos;
uniform vec2 invTextureSize;

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

uniform struct GBuffer
{
	sampler2D positionsSampler;
	sampler2D normalsSampler;
	sampler2D albedoOpacitySampler;
	sampler2D specularSampler;
	sampler2D emissiveSampler;
} gBuffer;

float ShadowCalculation(vec3 pos, float cosTheta)
{
	// texture(shadowMap, shadowCoord.xy).z is the distance between the light and the nearest occluder
	// shadowCoord.z is the distance between the light and the current fragment

	vec4 shadowCoord4 = spotLight.mvpBiased * vec4(pos, 1.0);

	
	// cure shadow acne
	float bias = spotLight.maxShadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, spotLight.maxShadowBias);

	float currentDepth = (shadowCoord4.z - bias);

	// if behind shadow map just return shadow
	if(currentDepth / shadowCoord4.w < 0.005)
		return 1.0;
		
	float shadow = 0;
	
	// Stratified Poisson Sampling
	for (int i = 0; i < spotLight.samples; ++i)
	{
		int index = int(16.0*random(vec4(pos,i)))%16;
		
		vec2 shadowCoord = shadowCoord4.xy + poissonDisk[index]/100.0;
		vec4 P = vec4(shadowCoord.xy, currentDepth, shadowCoord4.w);
		
		// Hardware implemented PCF on sample
		shadow += (1.0-textureProj(spotLight.shadowMap, P, 0.f));
	}
		
    return shadow / spotLight.samples;
}  

void ProcessUniformGBuffer(out vec3 albedo, out float opacity, out float metallic, 
out float roughness, out vec3 emissive, out float occlusion, out float occlusionStrength, 
out vec3 pos, out vec3 normal)
{
	vec2 uv = gl_FragCoord.st * invTextureSize;

	pos = texture(gBuffer.positionsSampler, uv).rgb;
	normal = texture(gBuffer.normalsSampler, uv).rgb;
	vec4 albedoOpacity = texture(gBuffer.albedoOpacitySampler, uv);
	albedo = albedoOpacity.rgb;
	opacity = albedoOpacity.a;
	vec4 metallicRoughnessOcclusionOcclusionStrength = texture(gBuffer.specularSampler, uv);
	metallic = metallicRoughnessOcclusionOcclusionStrength.r;
	roughness = metallicRoughnessOcclusionOcclusionStrength.g;
	occlusion = metallicRoughnessOcclusionOcclusionStrength.b;
	occlusionStrength = metallicRoughnessOcclusionOcclusionStrength.a;
	emissive = texture(gBuffer.emissiveSampler, uv).rgb;
}

void main()
{
	// gBuffer values
	vec3 albedo;
	float opacity;
	float metallic;
	float roughness;
	float occlusion;
	float occlusionStrength;
	vec3 emissive;
	vec3 wcs_fragPos;
	vec3 wcs_normal;
	
	ProcessUniformGBuffer(albedo, opacity, metallic, roughness, emissive,
		occlusion, occlusionStrength, wcs_fragPos, wcs_normal);
	
	// vectors are in wcs
	vec3 N = wcs_normal;
	vec3 V = normalize(wcs_viewPos - wcs_fragPos);
	vec3 L = normalize(spotLight.wcs_pos - wcs_fragPos); 
	
	// attenuation
	float distance = length(spotLight.wcs_pos - wcs_fragPos);
	float attenuation = 1.0 / pow(distance, spotLight.attenCoef);
	
    // spot effect (soft edges)
	float theta = dot(L, normalize(-spotLight.wcs_dir));
    float epsilon = (spotLight.innerCutOff - spotLight.outerCutOff);
    float spotEffect = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
	 
	float shadow = ShadowCalculation(wcs_fragPos, max(dot(N, L), 0.0)); 
	vec3 Li = ((1.0 - shadow) * spotLight.color * spotLight.intensity * attenuation * spotEffect); 

	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, albedo, metallic, roughness) * Li * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, occlusionStrength);
	
	out_color = vec4(color, opacity);
}