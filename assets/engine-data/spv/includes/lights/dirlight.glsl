#ifndef dirlight_glsl
#define dirlight_glsl

#include "bsdf.glsl"
#include "shadow.glsl"
#include "surface.glsl"

vec3 Dirlight_Contribution(Dirlight dl, sampler2DShadow shadowmap, Surface surface, float shadow)
{
	vec3 L = normalize(-dl.front); // explicit light dir
	addIncomingLightDirection(surface, L);

	vec3 Li = (1.0 - shadow) * dl.color * dl.intensity; 

	return DirectLightBRDF(surface)  * Li * surface.nol;
}

vec3 Dirlight_FastContribution(Dirlight dl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = dl.hasShadow != 0 ? ShadowCalculationFast(shadowmap, dl.viewProj, surface.position, dl.maxShadowBias) : 0;
	return Dirlight_Contribution(dl, shadowmap, surface, shadow);
}

vec3 Dirlight_SmoothContribution(Dirlight dl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = dl.hasShadow != 0 ? ShadowCalculation(shadowmap, dl.viewProj, surface.position, 
	dl.maxShadowBias, surface.nol, dl.samples, dl.sampleInvSpread) : 0;
	return Dirlight_Contribution(dl, shadowmap, surface, shadow);
}

#endif
