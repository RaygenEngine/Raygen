#ifndef spotlight_glsl
#define spotlight_glsl

#include "shading-math.glsl"
#include "shadowmap.glsl"
#include "surface.glsl"

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
