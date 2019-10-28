#version 460 core

#include "/engine-data/glsl/include/random.h"
#include "/engine-data/glsl/include/microfacet_bsdf.h"
#include "/engine-data/glsl/include/poisson.h"

out vec4 out_color;

in vec2 uv;

uniform vec3 wcs_viewPos;

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

uniform struct GBuffer
{
	sampler2D positionsSampler;
	sampler2D normalsSampler;
	sampler2D albedoOpacitySampler;
	sampler2D specularSampler;
	sampler2D depthSampler;
} gBuffer;

float ShadowCalculation(vec3 pos, float cosTheta)
{
	if(!punctualLight.castsShadow)
		return 0.0;

	// get vector between fragment position and light position
    vec3 fragToLight = pos - punctualLight.wcs_pos;
	
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

void ProcessUniformGBuffer(out vec3 albedo, out float opacity, out float metallic, 
out float roughness, out vec3 pos, out vec3 normal)
{
	pos = texture(gBuffer.positionsSampler, uv).rgb;
	normal = texture(gBuffer.normalsSampler, uv).rgb;
	vec4 albedoOpacity = texture(gBuffer.albedoOpacitySampler, uv);
	albedo = albedoOpacity.rgb;
	opacity = albedoOpacity.a;
	vec4 metallicRoughnessOcclusionOcclusionStrength = texture(gBuffer.specularSampler, uv);
	metallic = metallicRoughnessOcclusionOcclusionStrength.r;
	roughness = metallicRoughnessOcclusionOcclusionStrength.g;
}

void main()
{
	float currentDepth = texture(gBuffer.depthSampler, uv.xy).r;

	if(currentDepth == 1.0)
	{
		out_color = vec4(0,0,0,1);
		return;
	}

	// gBuffer values
	vec3 albedo;
	float opacity;
	float metallic;
	float roughness;
	vec3 wcs_fragPos;
	vec3 wcs_normal;
	
	ProcessUniformGBuffer(albedo, opacity, metallic, roughness, wcs_fragPos, wcs_normal);
	
	vec3 N = wcs_normal;
	vec3 V = normalize(wcs_viewPos - wcs_fragPos);
	vec3 L = normalize(punctualLight.wcs_pos - wcs_fragPos);  
	
	// attenuation
	float distance = length(punctualLight.wcs_pos - wcs_fragPos); 
	float attenuation = 1.0 / pow(distance, punctualLight.attenCoef);
	
	float shadow = ShadowCalculation(wcs_fragPos, max(dot(N, L), 0.0)); 
	vec3 Li = ((1.0 - shadow) * punctualLight.color * punctualLight.intensity * attenuation); 

	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, albedo, metallic, roughness) * Li * max(dot(N, L), 0.0);

	out_color = vec4(Lo, opacity);
}



