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
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} camera;


layout(set = 2, binding = 0) uniform samplerCube skyboxSampler;
layout(set = 2, binding = 1) uniform samplerCube irradianceSampler;

vec3 ReconstructWorldPosition(float depth)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 worldPos = camera.viewProjInv * clipPos;

	return worldPos.xyz / worldPos.w; // return world space pos xyz
}

// Attempts to emulate the movement of the camera while reconstructing the world position for the sky.
vec3 ReconstructWorldPosForSky(float depth)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 worldPos = camera.viewProjInv * clipPos;

	// Probably should include fov for some of these calculations
	const float maxMovementPercentage = 0.6;
	vec3 movementEffect = camera.position / 500.0; // Units in world space. Max Movement occurs in 500*maxMovePerc to avoid extreme fish eye effect.
	movementEffect.x = clamp(movementEffect.x, -maxMovementPercentage, maxMovementPercentage);
	movementEffect.y = clamp(movementEffect.y, -maxMovementPercentage, maxMovementPercentage);
	movementEffect.z = clamp(movementEffect.z, -maxMovementPercentage, maxMovementPercentage);
	
	worldPos += vec4(movementEffect, 0);
	
	return worldPos.xyz / worldPos.w; // return world space pos xyz
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

vec3 ApplyGammaExposure(vec3 color)
{
	// gamma correction / exposure
	const float gamma = 2.2;
	
	const float exposure = 2.5;

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    // Gamma correction 
    return pow(mapped, vec3(1.0 / gamma));
	//return color;
} 

void main( ) {

	float currentDepth = texture(depthSampler, uv).r;

	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	vec3 V = normalize(ReconstructWorldPosition(currentDepth) - camera.position);

	if(currentDepth == 1.0)
	{
		// ART:
		V = normalize(ReconstructWorldPosForSky(currentDepth) - camera.position);
		outColor = vec4(ApplyGammaExposure(texture(skyboxSampler, V).rgb), 1.0);
		return;
	}
	
	
	vec3 N = normalize(texture(normalsSampler, uv).rgb);

	vec3 emissive = texture(emissiveSampler, uv).rgb;
	vec4 specular = texture(specularSampler, uv);
	vec3 albedo = texture(albedoOpacitySampler, uv).rgb;
	
	float metallic = specular.r;
	float roughness = specular.g;
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness); 
	vec3 kD = 1.0 - kS;
	vec3 irradiance = texture(irradianceSampler, N).rgb;
	vec3 diffuse    = irradiance * albedo;
	vec3 ambient    = (kD * diffuse);

	// AO
	vec3 color = emissive + ambient;
	color = mix(color, color * specular.b, specular.a);

	

  
	outColor = vec4(ApplyGammaExposure(color), 1.0);
}


