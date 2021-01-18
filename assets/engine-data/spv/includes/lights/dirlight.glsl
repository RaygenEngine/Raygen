#ifndef dirlight_glsl
#define dirlight_glsl

#include "radiance.glsl"
#include "shading-math.glsl"
#include "shadowmap.glsl"
#include "surface.glsl"

vec3 Dirlight_FastContribution(Dirlight dl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = dl.hasShadow != 0 ? ShadowCalculationFast(shadowmap, dl.viewProj, surface.position, dl.maxShadowBias) : 0;
	return Dirlight_Contribution(dl, shadowmap, surface, shadow);
}

vec3 Dirlight_SmoothContribution(Dirlight dl, sampler2DShadow shadowmap, Surface surface)
{
	//float shadow = dl.hasShadow != 0 ? ShadowCalculation(shadowmap, dl.viewProj, surface.position, 
	//dl.maxShadowBias, surface.nol, dl.samples, dl.sampleInvSpread) : 0;
	return vec3(0);//Dirlight_Contribution(dl, shadowmap, surface, shadow);
}

#endif
