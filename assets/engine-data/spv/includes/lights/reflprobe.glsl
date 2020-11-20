#ifndef reflprobe_glsl
#define reflprobe_glsl

#include "fresnel.glsl"
#include "surface.glsl"

vec3 Reflprobe_Contribution(Reflprobe rp, sampler2D brdfLutSampler, samplerCube irradianceSampler, samplerCube prefilteredSampler, Surface surface)
{		
	vec3 R = outOnbSpace(surface.basis, reflect(-surface.v));
	
	// CHECK: roughness / a differences
	float lod = sqrt(surface.a) * rp.lodCount; 
	
	// WIP: accesible 
	vec3 brdfLut = (texture(brdfLutSampler, vec2(surface.nov, surface.a))).rgb;

	vec3 ks = F_SchlickRoughness(surface.nov, surface.f0, surface.a);
	vec3 kd = 1.0 - ks;

	vec3 diffuseLight = texture(irradianceSampler, surface.basis.normal).rgb;

	
	vec3 specularLight = textureLod(prefilteredSampler, R, lod).rgb;

	vec3 diffuse = diffuseLight * surface.albedo;
	vec3 specular = specularLight * (surface.f0 * brdfLut.x + brdfLut.y);

	return kd * diffuse + ks * specular;
}

#endif
