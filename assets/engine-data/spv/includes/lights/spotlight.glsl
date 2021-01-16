#ifndef spotlight_glsl
#define spotlight_glsl

#include "shading-math.glsl"
#include "shadowmap.glsl"
#include "surface.glsl"

vec3 Spotlight_Contribution(Spotlight sl, sampler2DShadow shadowmap, Surface surface, float shadow)
{
	vec3 L = normalize(sl.position - surface.position);

	float dist = distance(sl.position, surface.position);
	float attenuation = 1.0 / (sl.constantTerm + sl.linearTerm * dist + 
  			     sl.quadraticTerm * (dist * dist));

	// spot effect (soft edges)
	float theta = dot(L, -sl.front);
    float epsilon = (sl.innerCutOff - sl.outerCutOff);
    float spotEffect = clamp((theta - sl.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 Li = (1.0 - shadow) * sl.color * sl.intensity * attenuation * spotEffect; 
	return Li * explicitBrdf(surface, L);
}

vec3 Spotlight_FastContribution(Spotlight sl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = sl.hasShadow != 0 ? ShadowCalculationFast(shadowmap, sl.viewProj, surface.position, sl.maxShadowBias) : 0;
	return Spotlight_Contribution(sl, shadowmap, surface, shadow);
}

vec3 Spotlight_SmoothContribution(Spotlight sl, sampler2DShadow shadowmap, Surface surface)
{
	//float shadow = sl.hasShadow != 0 ? ShadowCalculation(shadowmap, sl.viewProj, surface.position, 
	///sl.maxShadowBias, surface.nol, sl.samples, sl.sampleInvSpread) : 0;
	return vec3(0);//Spotlight_Contribution(sl, shadowmap, surface, shadow);
}

#endif
