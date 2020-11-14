#ifndef shadow_glsl
#define shadow_glsl

#include "poisson.glsl"
#include "random.glsl"

float ShadowCalculation(sampler2DShadow shadowmap, mat4 lightMatrix, vec3 fragPos, float maxBias, float cosTheta, int samples, float invSpread)
{
	vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
 	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
 	
	float bias = maxBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, maxBias);
	
	projCoords = vec3(projCoords.xy, projCoords.z - bias);

	float shadow = 0;
	// Stratified Poisson Sampling
	// PERF: samples here probably prevent loop unrolling
	for (int i = 0; i < samples; ++i)
	{
		int index = int(16.0*rand(vec4(fragPos, i)))%16;
		
		vec3 jitteredProjCoords = vec3((projCoords.xy + poissonDisk16[index] / invSpread) * 0.5 + 0.5, projCoords.z);
	
		// Hardware implemented PCF on sample
		shadow += 1.0 - texture(shadowmap, jitteredProjCoords);
	}
		
    return shadow / samples;
} 

float ShadowCalculationFast(sampler2DShadow shadowmap, mat4 lightMatrix, vec3 fragPos, float bias)
{
	vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
 	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
 	
	projCoords = vec3(projCoords.xy * 0.5 + 0.5, projCoords.z - bias);

    return 1.0 - texture(shadowmap, projCoords);
} 

#endif
