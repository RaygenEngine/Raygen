#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive: enable

#include "microfacet_bsdf.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 0, binding = 0) uniform sampler2D positionsSampler;
layout(set = 0, binding = 1) uniform sampler2D normalsSampler;
layout(set = 0, binding = 2) uniform sampler2D albedoOpacitySampler;
layout(set = 0, binding = 3) uniform sampler2D specularSampler;
layout(set = 0, binding = 4) uniform sampler2D emissiveSampler;
layout(set = 0, binding = 5) uniform sampler2D depthSampler;

layout(set = 1, binding = 0) uniform UBO_Camera {
    vec4 position;
	mat4 viewProj;
} camera;

layout(set = 2, binding = 0) uniform UBO_Spotlight {
		vec4 position;
		vec4 direction;

		// Lightmap
		mat4 viewProj;
		vec4 color;

		float intensity;

		float near;
		float far;

		float outerCutOff;
		float innerCutOff;
} light;

// globals

struct Fragment
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
    float opacity;
    vec3 emissive;
    float metallic;
    float roughness;
} fragment;

void main() {

    float currentDepth = texture(depthSampler, uv).r;

	if(currentDepth == 1.0)
	{
	    outColor = vec4(0,0,0.4,1);
		return;
	}

    fragment.position = texture(positionsSampler, uv).rgb;
    fragment.normal = texture(normalsSampler, uv).rgb;
    vec4 albedoOpacity = texture(albedoOpacitySampler, uv);
    fragment.albedo = albedoOpacity.rgb;
    fragment.opacity = albedoOpacity.a;
    fragment.emissive = texture(emissiveSampler, uv).rgb;
    vec4 metallicRoughnessOcclusionOcclusionStrength = texture(specularSampler, uv);
    fragment.metallic = metallicRoughnessOcclusionOcclusionStrength.r;
    fragment.roughness = metallicRoughnessOcclusionOcclusionStrength.g;

	// spot light
	vec3 N = fragment.normal;
	vec3 V = normalize(camera.position.rgb - fragment.position);
	vec3 L = normalize(light.position.rgb - fragment.position); 
	
	// attenuation
	float dist = length(light.position.rgb - fragment.position);
	float attenuation = 1.0 / pow(dist, /*____attenuation coef______*/ 1.f);
	
    // spot effect (soft edges)
	float theta = dot(L, normalize(-light.direction.rgb));
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	 
	//float shadow = ShadowCalculation(wcs_fragPos, max(dot(N, L), 0.0)); 
	vec3 Li = /*((1.0 - shadow) **/ light.color.rgb * light.intensity * attenuation * spotEffect; 

	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, fragment.albedo, fragment.metallic, fragment.roughness) * Li * max(dot(N, L), 0.0);

    // TODO: emissive at a later pass
    vec3 hdrColor = Lo + fragment.emissive;

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.2);
    // Gamma correction 
	mapped = pow(mapped, vec3(1.0 / 2.2));
  
    outColor = vec4(mapped, fragment.opacity);
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
