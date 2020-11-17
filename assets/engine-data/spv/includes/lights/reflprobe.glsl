#ifndef reflprobe_glsl
#define reflprobe_glsl

#include "fresnel.glsl"
#include "surface.glsl"

vec3 Reflprobe_Contribution(samplerCube irradianceSampler, samplerCube prefilteredSampler, int lodCount, Surface surface)
{										  
	vec3 N = surface.basis.normal;
	vec3 R = normalize(reflect(-surface.v));
	
	// CHECK: roughness / a differences
	float lod = surface.a * lodCount; 
	
	vec3 brdfLut = vec3(1,1,1);// WIP:  (texture(std_BrdfLut, vec2(surface.nov, surface.a))).rgb;

	vec3 ks = F_SchlickRoughness(surface.nov, surface.f0, surface.a);
	vec3 kd = 1.0 - ks;

	vec3 diffuseLight = texture(irradianceSampler, N).rgb;
	vec3 specularLight = textureLod(prefilteredSampler, R, lod).rgb;

	vec3 diffuse = diffuseLight * surface.albedo;
	vec3 specular = specularLight * (surface.f0 * brdfLut.x + brdfLut.y);

	return kd * diffuse + ks * specular;
}

#endif
