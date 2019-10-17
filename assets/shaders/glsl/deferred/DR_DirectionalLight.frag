#version 460 core

#include "/shaders/glsl/include/random.h"
#include "/shaders/glsl/include/microfacet_bsdf.h"
#include "/shaders/glsl/include/poisson.h"

out vec4 out_color;

in vec2 uv;

uniform vec3 wcs_viewPos;
uniform vec2 invTextureSize;

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

	vec4 shadowCoord4 = directionalLight.mvpBiased * vec4(pos, 1.0);

	// cure shadow acne
	float bias = directionalLight.maxShadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, directionalLight.maxShadowBias);

	float currentDepth = shadowCoord4.z;
	vec2 shadowCoord = shadowCoord4.xy;

	// if behind shadow map just return shadow
	if(currentDepth < 0.005)
		return 1.0;

	float shadow = 0;
	
	// Stratified Poisson Sampling
	for (int i = 0; i < directionalLight.samples; ++i)
	{
		int index = int(16.0*random(vec4(pos,i)))%16;
		
		// Hardware implemented PCF on sample
		shadow += (1.0-texture(directionalLight.shadowMap, 
		vec3(shadowCoord + poissonDisk[index]/1400.0,  (currentDepth-bias))));
	}
		
    return shadow / directionalLight.samples;
}  

void ProcessUniformGBuffer(out vec3 albedo, out float opacity, out float metallic, 
out float roughness, out vec3 emissive, out float occlusion, out float occlusionStrength, 
out vec3 pos, out vec3 normal)
{
	// material values
	pos = texture(gBuffer.positionsSampler, gl_FragCoord.st * invTextureSize).rgb;
	normal = texture(gBuffer.normalsSampler, gl_FragCoord.st * invTextureSize).rgb;
	vec4 albedoOpacity = texture(gBuffer.albedoOpacitySampler, gl_FragCoord.st * invTextureSize);
	albedo = albedoOpacity.rgb;
	opacity = albedoOpacity.a;
	vec4 metallicRoughnessOcclusionOcclusionStrength = texture(gBuffer.specularSampler, gl_FragCoord.st * invTextureSize);
	metallic = metallicRoughnessOcclusionOcclusionStrength.r;
	roughness = metallicRoughnessOcclusionOcclusionStrength.g;
	occlusion = metallicRoughnessOcclusionOcclusionStrength.b;
	occlusionStrength = metallicRoughnessOcclusionOcclusionStrength.a;
	emissive = texture(gBuffer.emissiveSampler, gl_FragCoord.st * invTextureSize).rgb;
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
	vec3 L = normalize(-directionalLight.wcs_dir); 
	vec3 H = normalize(V + L);
	
	float shadow = ShadowCalculation(wcs_fragPos, max(dot(N, L), 0.0)); 
	vec3 Li = (1.0 - shadow) * directionalLight.color * directionalLight.intensity; 
	
	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, albedo, metallic, roughness) * Li * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, occlusionStrength);
		
	out_color = vec4(vec3(color), opacity);
}