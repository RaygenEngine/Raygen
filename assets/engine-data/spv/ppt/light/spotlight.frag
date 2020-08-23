#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: enable
#include "global.h"

#include "bsdf.h"
#include "fragment.h"
#include "shadow-sampling.h"


//#define RTX_ON

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
} cam;

layout(set = 2, binding = 0) uniform UBO_Spotlight {
		vec3 position;
		float pad0;
		vec3 direction;
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

#ifdef RTX_ON
layout(set = 4, binding = 0) uniform accelerationStructureEXT topLevelAS;
#endif

void main() {

	float depth = texture(depthSampler, uv).r;

	if(depth == 1.0)
	{
		discard;
	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		normalsSampler,
		baseColorSampler,
		surfaceSampler,
		emissiveSampler,
		uv);
		
	// spot light
	vec3 N = frag.normal;
	vec3 V = normalize(cam.position - frag.position);
	vec3 L = normalize(light.position - frag.position); 
	
	// attenuation
	float dist = length(light.position - frag.position);
	float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
  			     light.quadraticTerm * (dist * dist));
	
    // spot effect (soft edges)
	float theta = dot(L, -light.direction);
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	float NoL = saturate(dot(N, L));

	float shadow;
	//float shadow = ShadowCalculationFast(shadowmap, light.viewProj, frag.position, light.maxShadowBias);


	
#ifndef RTX_ON
	shadow = ShadowCalculation(shadowmap, light.viewProj, frag.position, light.maxShadowBias, NoL, light.samples, light.sampleInvSpread);
#else
	vec3 L = normalize(light.position - frag.position); 
	vec3  origin    = frag.position;
	vec3  direction = L;  // vector to light
	float tMin      = 0.01f;
	float tMax      = distance(frag.position, light.position);

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin,
                      direction, tMax);

	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery)) {
	}
      
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
	  // Got an intersection == Shadow
	  shadow = 1.0;
	}
	else{
		shadow = 0.0;
	}
#endif
	
	vec3 Li = (1.0 - shadow) * light.color * light.intensity * attenuation * spotEffect; 
	
	vec3 H = normalize(V + L);

    float NoV = abs(dot(N, V)) + 1e-5; 
    float NoH = saturate(dot(N, H));
    float LoH = saturate(dot(L, H));

	// to get final diffuse and specular both those terms are multiplied by Li * NoL
	vec3 brdf_d = Fd_Burley(NoV, NoL, LoH, frag.diffuseColor, frag.a);
	vec3 brdf_r = Fr_CookTorranceGGX(NoV, NoL, NoH, LoH, frag.f0, frag.a);

	// so to simplify (faster math)
	vec3 finalContribution = (brdf_d + brdf_r) * Li * NoL;

    outColor = vec4(finalContribution, 1);
    
 
}                               





