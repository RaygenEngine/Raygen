#ifndef reflprobe_glsl
#define reflprobe_glsl

#include "aabb.glsl"
#include "surface.glsl"

// PERF:
float hit_sphere(vec3 center, float radius, vec3 orig, vec3 dir)
{
	vec3 oc = orig - center;
	float a = dot(dir, dir);
	float b = 2.0 * dot(oc, dir);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0.0) {
		return -1.0;
	}
	else {
		float numerator = -b - sqrt(discriminant);
		if (numerator
			> 0.0) { return numerator / (2.0 * a); }

		numerator = -b + sqrt(discriminant);
		if (numerator
			> 0.0) { return numerator / (2.0 * a); }
		else {
			return -1;
		}
	}
}

vec3 Reflprobe_Contribution(Reflprobe rp, sampler2D brdfLutSampler, samplerCube irradianceSampler, samplerCube prefilteredSampler, Surface surface)
{	
	if(distance(rp.position, surface.position) > rp.radius) {
		return vec3(0);
	}

	vec3 R = outOnbSpace(surface.basis, reflect(-surface.i));
	vec3 reprojR = (surface.position - rp.position) + hit_sphere(rp.position, rp.radius, surface.position,  R) *  R;

	// CHECK: roughness / a differences
	float lod = sqrt(surface.a) * rp.lodCount; 
	
	// TODO: brdf lut accesible from global desc set
	vec3 brdfLut = vec3(0);//(texture(brdfLutSampler, vec2(surface.nov, surface.a))).rgb;

	vec3 ks = vec3(0);//F_SchlickRoughness(surface.nov, surface.f0, surface.a);
	vec3 kd = 1.0 - ks;

	vec3 diffuseLight = texture(irradianceSampler, surface.basis.normal).rgb;

	vec3 specularLight = textureLod(prefilteredSampler, reprojR, lod).rgb;

//	vec3 diffuse = diffuseLight * DiffuseTerm(surface) * rp.irradianceFactor;
	vec3 specular = specularLight * (surface.f0 * brdfLut.x + brdfLut.y);

	//  kd * diffuse +
	return ks * specular;
}

#endif
