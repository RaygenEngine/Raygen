#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive: enable

#include "microfacet_bsdf.h"
#include "fragment.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 0, binding = 0) uniform sampler2D normalsSampler;
layout(set = 0, binding = 1) uniform sampler2D baseColorSampler;
layout(set = 0, binding = 2) uniform sampler2D surfaceSampler;
layout(set = 0, binding = 3) uniform sampler2D emissiveSampler;
layout(set = 0, binding = 4) uniform sampler2D depthSampler;

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

vec3 ReconstructWorldPositionTEMP(float depth)
{
	// clip space reconstruction
	vec4 clipPos; 
	clipPos.xy = uv.xy * 2.0 - 1;
	clipPos.z = depth;
	clipPos.w = 1.0;
	
	vec4 worldPos = camera.viewProjInv * clipPos;

	return worldPos.xyz / worldPos.w; // return world space pos xyz
}

vec4 SampleCubemapLH(samplerCube cubemap, vec3 RHdirection) {
	return texture(cubemap, vec3(RHdirection.x, RHdirection.y, -RHdirection.z));
}

void main( ) {

	float depth = texture(depthSampler, uv).r;

	if(depth == 1.0)
	{
		// TODO: discard here like in spotlights
		vec3 V = normalize(ReconstructWorldPositionTEMP(depth) - camera.position);
		outColor = SampleCubemapLH(skyboxSampler, V);
		
		return;
	}

	// PERF:
	Fragment fragment = GetFragmentFromGBuffer(
		depth,
		camera.viewProjInv,
		normalsSampler,
		baseColorSampler,
		surfaceSampler,
		emissiveSampler,
		uv);
	
	
	vec3 N = fragment.normal;
	
	vec3 diffuseColor = (1.0 - fragment.metallic) * fragment.baseColor;
	vec3 f0 = 0.16 * fragment.reflectance * fragment.reflectance * (1.0 - fragment.metallic) + fragment.baseColor * fragment.metallic;
	float a = fragment.roughness * fragment.roughness;
	
	vec3 V = normalize(fragment.position - camera.position);
	vec3 reflection = normalize(reflect(V, N));
	
	float NdotV = abs(dot(N, V)) + 1e-5;
	
	// Actual IBL Contribution
	const float MAX_REFLECTION_LOD = 4.0;
	float lod = (a * MAX_REFLECTION_LOD); 
	
	vec3 brdf = (texture(brdfLutSampler, vec2(NdotV, a))).rgb;
	vec3 diffuseLight = texture(irradianceSampler, N).rgb;

	vec3 specularLight = textureLod(prefilteredSampler, reflection, lod).rgb;

	vec3 diffuse = diffuseLight * diffuseColor;
	vec3 specular = specularLight * (f0 * brdf.x + brdf.y);


	vec3 iblContribution = diffuse + specular;

	outColor =  vec4(iblContribution, 1.0f);
}

