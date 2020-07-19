#ifndef shadow_sampling_h
#define shadow_sampling_h

#include "random.h"
#include "poisson.h"

float ShadowCalculation(in sampler2DShadow shadowmap, mat4 lightMatrix, vec3 fragPos, float maxBias, float NoL, int samples, float invSpread)
{
	vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
 	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
 	
	float bias = maxBias * tan(acos(NoL));
	bias = clamp(bias, 0.0, maxBias);
	
	projCoords = vec3(projCoords.xy * 0.5 + 0.5, projCoords.z - bias);

	float shadow = 0;
		
	// Stratified Poisson Sampling
	for (int i = 0; i < samples; ++i)
	{
		int index = int(16.0*random(vec4(fragPos.xyz,i)))%16;
		
		vec3 jitteredProjCoords = vec3(projCoords.xy + poissonDisk[index] / invSpread, projCoords.z);
	
		// Hardware implemented PCF on sample
		shadow += 1.0 - texture(shadowmap, jitteredProjCoords, 0.f);
	}
		
    return shadow / samples;
} 

float ShadowCalculationFast(in sampler2DShadow shadowmap, mat4 lightMatrix, vec3 fragPos, float bias)
{
	vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
 	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
 	
	projCoords = vec3(projCoords.xy * 0.5 + 0.5, projCoords.z - bias);

    return 1.0 - texture(shadowmap, projCoords, 0.f);
} 

#endif