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

void main( ) {

	float currentDepth = texture(depthSampler, uv).r;

	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	vec3 I = normalize(ReconstructWorldPosition(currentDepth) - camera.position);

	if(currentDepth == 1.0)
	{
		// ART:
		I = normalize(ReconstructWorldPosForSky(currentDepth) - camera.position);
		outColor = texture(skyboxSampler, I);
		return;
	}
	
	vec3 N = normalize(texture(normalsSampler, uv).rgb);
    vec3 R = reflect(I, N);
    
    vec3 reflColor = texture(skyboxSampler, R).rgb;

	vec3 emissive = texture(emissiveSampler, uv).rgb;
	vec4 specular = texture(specularSampler, uv);
	vec3 albedo = texture(albedoOpacitySampler, uv).rgb;
	
	float metallic = specular.r;
	float roughness = specular.g;
	

	

	
	// ART:
	vec3 specColor = mix(texture(irradianceSampler, N).rgb * albedo, reflColor, 1-roughness);
	
	
	vec3 color = emissive + specColor;
	color = mix(color, color * specular.b, specular.a);

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	vec3 kS = fresnelSchlickRoughness(max(dot(N, I), 0.0), F0, roughness); 
	vec3 kD = 1.0 - kS;
	vec3 irradiance = texture(irradianceSampler, N).rgb;
	vec3 diffuse    = irradiance * albedo;
	vec3 ambient    = (kD * diffuse);

	
	outColor += vec4(ambient, 1);
	
	outColor += outColor / 4.0f;
}                                                                                                                          
                                                                                                                                       
                                                                                                                                                                 
                                                                                                                       
                                                                                                                     
                                                                                                                      
                                                                                                                        
                                                                                                                         
