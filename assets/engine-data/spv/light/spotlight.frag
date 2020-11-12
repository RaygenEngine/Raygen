#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

#include "surface.glsl"
#include "shadow.glsl"
#include "bsdf.glsl"
#include "onb.glsl"
#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform UBO_Spotlight { Spotlight light; };
layout(set = 3, binding = 0) uniform sampler2DShadow shadowmap;

void main() {

//	float depth = texture(g_DepthSampler, uv).r;
//
//	if(depth == 1.0) {
//		discard;
//	}
//
//	// PERF:
//	Fragment frag = getFragmentFromGBuffer(
//		depth,
//		cam.viewProjInv,
//		g_NormalSampler,
//		g_AlbedoSampler,
//		g_SpecularSampler,
//		g_EmissiveSampler,
//		uv);
//		
//		
//
//	Onb shadingOrthoBasis = branchlessOnb(frag.normal);
//	
//	vec3 V = normalize(cam.position - frag.position);
//	vec3 L = normalize(light.position - frag.position);
//	vec3 lDir = -light.front;
//
//	toOnbSpace(shadingOrthoBasis, V);
//	toOnbSpace(shadingOrthoBasis, L);
//	toOnbSpace(shadingOrthoBasis, lDir);  
//	
//	// attenuation
//	float dist = length(light.position - frag.position);
//	float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
//  			     light.quadraticTerm * (dist * dist));
//	
//    // spot effect (soft edges)
//	float theta = dot(L, lDir);
//    float epsilon = (light.innerCutOff - light.outerCutOff);
//    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
//	
//	vec3 H = normalize(V + L); 
//	float NoL = max(Ndot(L), BIAS);
//	float NoV = max(Ndot(V), BIAS);
//	float NoH = max(Ndot(H), BIAS); 
//	float LoH = max(dot(L, H), BIAS);
//
//	float shadow = ShadowCalculation(shadowmap, light.viewProj, frag.position, 
//	light.maxShadowBias, NoL, light.samples, light.sampleInvSpread);
//
//	vec3 Li = (1.0 - shadow) * light.color * light.intensity * attenuation * spotEffect; 
//
//	// Li comes from direct light path
//	vec3 finalContribution = DirectLightBRDF(NoL, NoV, NoH, LoH, frag.a, frag.albedo, frag.f0)  * Li * NoL;
//
//	outColor = vec4(finalContribution, 1);
}                               



















