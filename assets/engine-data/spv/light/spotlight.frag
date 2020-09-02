#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require
#include "global.h"

#include "fragment.h"
#include "shadow.h"
#include "sampling.h"
#include "bsdf.h"
#include "onb.h"
#include "attachments.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj; 
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} cam;

layout(set = 2, binding = 0) uniform UBO_Spotlight {
		vec3 position;
		float pad0;
		vec3 front;
		float pad1;

		// CHECK: could pass this mat from push constants (is it better tho?)
		// Lightmap
		mat4 viewProj;
		vec3 color;
		float pad3;

		float intensity;

		float near;
		float far;

		float outerCutOff;
		float innerCutOff;

		float constantTerm;
		float linearTerm;
		float quadraticTerm;

		float maxShadowBias;
		int samples;
		float sampleInvSpread;
} light;

layout(set = 3, binding = 0) uniform sampler2DShadow shadowmap;
layout(set = 4, binding = 0) uniform accelerationStructureEXT topLevelAs;

float getShadowRayQuery(Fragment frag){ 
	vec3  L = normalize(light.position - frag.position); 
	vec3  origin    = frag.position;
	vec3  direction = L;  // vector to light
	float tMin      = 0.01f;
	float tMax      = distance(frag.position, light.position);

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAs, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin,
                      direction, tMax);

	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery)) {
	}
      
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
	  // Got an intersection == Shadow
	  return 1.0;
	}
	return 0.0;
}

void main() {

	float depth = texture(g_DepthSampler, uv).r;

	if(depth == 1.0)
	{
		discard;
	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		g_NormalSampler,
		g_ColorSampler,
		g_MRROSampler,
		g_EmissiveSampler,
		uv);

	Onb shadingOrthoBasis = branchlessOnb(frag.normal);

	vec3 wo = normalize(cam.position - frag.position);
	vec3 wi = normalize(light.position - frag.position);
	vec3 lDir = -light.front;

	toOnbSpace(shadingOrthoBasis, wo);
	toOnbSpace(shadingOrthoBasis, wi);
	toOnbSpace(shadingOrthoBasis, lDir);  
	
	// attenuation
	float dist2 = dot(wi, wi);
	float attenuation = 1.0 / (light.constantTerm + light.linearTerm * sqrt(dist2) + 
  			     light.quadraticTerm * dist2);
	
    // spot effect (soft edges)
	float theta = dot(wi, lDir);
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	float cosTheta = CosTheta(wi);

	// TODO: missing reflect
	outColor = vec4(vec3(0), 1);
	if(cosTheta > 0)
	{

		float shadow;

	#define RTX_ON
		
	#ifndef RTX_ON
		shadow = ShadowCalculation(shadowmap, light.viewProj, frag.position, light.maxShadowBias, cosTheta, light.samples, light.sampleInvSpread);
	#else
		shadow = getShadowRayQuery(frag);
	#endif
		
		vec3 Li = (1.0 - shadow) * light.color * light.intensity * attenuation * spotEffect; 
	
		// to get final diffuse and specular both those terms are multiplied by Li * NoL
		vec3 brdf_d = LambertianReflection(frag.diffuseColor);
		vec3 brdf_r = MicrofacetReflection(wo, wi, frag.a, frag.a, frag.f0);

		// so to simplify (faster math)
		// throughput = (brdf_d + brdf_r) * NoL
		// incoming radiance = Li;
		vec3 finalContribution = (brdf_d + brdf_r) * Li * cosTheta;

		outColor = vec4(finalContribution, 1);
	}
}                               





