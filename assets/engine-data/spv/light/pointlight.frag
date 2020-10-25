#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require

#include "global.glsl"

#include "fragment.glsl"
#include "sampling.glsl"
#include "bsdf.glsl"
#include "onb.glsl"
#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

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

layout(set = 2, binding = 0) uniform UBO_Pointlight {
		vec3 position;
		float pad0;

		vec3 color;
		float pad3;

		float intensity;

		float constantTerm;
		float linearTerm;
		float quadraticTerm;
} light;



layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;

float ShadowRayQuery(Fragment frag){ 
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

void main()
{
	vec2 iuv = gl_FragCoord.xy;
	ivec2 screenSize = textureSize(g_AlbedoSampler, 0);

	vec2 uv = iuv / screenSize; 

	float depth = texture(g_DepthSampler, uv).r;

	if(depth == 1.0) {
		discard;
	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv);

	Onb shadingOrthoBasis = branchlessOnb(frag.normal);
	
	vec3 V = normalize(cam.position - frag.position);
	vec3 L = normalize(light.position - frag.position);

	toOnbSpace(shadingOrthoBasis, L);	
	
	float NoL = Ndot(L);
	if(NoL < BIAS) {
		discard;
	}

	toOnbSpace(shadingOrthoBasis, V);

	
	// attenuation
	float dist = length(light.position - frag.position);
	float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
  			     light.quadraticTerm * (dist * dist));
	
	vec3 H = normalize(V + L); 

	float NoV = max(Ndot(V), BIAS);
	float NoH = max(Ndot(H), BIAS); 
	float LoH = max(dot(L, H), BIAS);

	float shadow = ShadowRayQuery(frag);

	vec3 Li = (1.0 - shadow) * light.color * light.intensity * attenuation; 

	// Li comes from direct light path
	vec3 finalContribution = DirectLightBRDF(NoL, NoV, NoH, LoH, frag.a, frag.albedo, frag.f0)  * Li * NoL;

	outColor = vec4(finalContribution, 1);
}                               


















