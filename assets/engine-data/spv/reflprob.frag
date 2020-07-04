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
layout(set = 2, binding = 2) uniform samplerCube prefilteredSampler;
layout(set = 2, binding = 3) uniform sampler2D brdfLutSampler;

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

vec4 SampleCubemapLH(samplerCube cubemap, vec3 RHdirection) {
	return texture(cubemap, vec3(RHdirection.x, RHdirection.y, -RHdirection.z));
}

void main( ) {

	float currentDepth = texture(depthSampler, uv).r;

	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	

	if(currentDepth == 1.0)
	{
		// ART:
		vec3 V = normalize(ReconstructWorldPosForSky(currentDepth) - camera.position);
		outColor = SampleCubemapLH(skyboxSampler, V);
		
		//outColor = texture(brdfLutSampler, uv).rgba;
		//outColor = mix(outColor, textureLod(prefilteredSampler, V,  0), 0); 
		//outColor = vec4(ReconstructWorldPosition(currentDepth),1.f);
		return;
	}
	
	vec3 N = normalize(texture(normalsSampler, uv).rgb);

	vec3 emissive = texture(emissiveSampler, uv).rgb;
	vec4 specularMat = texture(specularSampler, uv);
	vec3 baseColor = texture(albedoOpacitySampler, uv).rgb;
	
	float metallic = specularMat.r;
	float roughness = specularMat.g;
	
	vec3 f0 = vec3(0.04);
	vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;
	
	vec3 specularColor = mix(f0, baseColor.rgb, metallic);
	
	vec3 V = normalize(ReconstructWorldPosition(currentDepth) - camera.position);
	vec3 reflection = normalize(reflect(V, N));
	
	float NdotV = clamp(abs(dot(N, V)), 0.001, 1.0);
	
	// Actual IBL Contribution
	const float MAX_REFLECTION_LOD = 4.0;
	float lod = (roughness * MAX_REFLECTION_LOD); 
	
	vec3 brdf = (texture(brdfLutSampler, vec2(NdotV, roughness))).rgb;
	vec3 diffuseLight = texture(irradianceSampler, N).rgb;

	vec3 specularLight = textureLod(prefilteredSampler, reflection, lod).rgb;

	vec3 diffuse = diffuseLight * diffuseColor;
	vec3 specular = specularLight * (specularColor * brdf.x + brdf.y);


	vec3 iblContribution = diffuse + specular;

	// TODO: Temporary emissive hack
	vec3 color = iblContribution + emissive;

	outColor = vec4(color, 1.0f);
}

