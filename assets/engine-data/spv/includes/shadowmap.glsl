#ifndef shadow_glsl
#define shadow_glsl

#include "random.glsl"

float ShadowCalculation(sampler2DShadow shadowmap, mat4 lightMatrix, vec3 fragPos, float maxBias, float cosTheta, int samples, float invSpread)
{
	// CHECK:
	const vec2 poissonDisk16[16] = vec2[](
		vec2(-0.94201624, -0.39906216),
		vec2(0.94558609, -0.76890725),
		vec2(-0.094184101, -0.92938870),
		vec2(0.34495938, 0.29387760),
		vec2(-0.91588581, 0.45771432),
		vec2(-0.81544232, -0.87912464),
		vec2(-0.38277543, 0.27676845),
		vec2(0.97484398, 0.75648379),
		vec2(0.44323325, -0.97511554),
		vec2(0.53742981, -0.47373420),
		vec2(-0.26496911, -0.41893023),
		vec2(0.79197514, 0.19090188),
		vec2(-0.24188840, 0.99706507),
		vec2(-0.81409955, 0.91437590),
		vec2(0.19984126, 0.78641367),
		vec2(0.14383161, -0.14100790)
	);

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
