#version 450 
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "bsdf.glsl"
#include "fragment.glsl"
#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} cam;


layout(set = 2, binding = 0) uniform samplerCube skyboxSampler;
layout(set = 2, binding = 1) uniform samplerCube irradianceSampler;
layout(set = 2, binding = 2) uniform samplerCube prefilteredSampler;
layout(set = 2, binding = 3) uniform sampler2D brdfLutSampler;

void main( ) {

	float depth = texture(g_DepthSampler, uv).r;

	// TODO: discard when skymesh is implemented
	if(depth == 1.0) {
		// TODO: discard here like in spotlights
		vec3 V = normalize(reconstructWorldPosition(depth, uv, cam.viewProjInv) - cam.position);
		outColor = sampleCubemapLH(skyboxSampler, V);
		
		return;
	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv);
	
	
	vec3 N = frag.normal;
	vec3 V = normalize(frag.position - cam.position);
	vec3 R = normalize(reflect(V, N));

    float NoV = abs(dot(N, V)) + 1e-5;
	
	const float MAX_REFLECTION_LOD = 4.0;
	// CHECK: which roughness should go here
	float lod = (frag.a * MAX_REFLECTION_LOD); 
	
	vec3 brdf = (texture(brdfLutSampler, vec2(NoV, frag.a))).rgb;

	// CHECK: math of those
	vec3 diffuseLight = texture(irradianceSampler, N).rgb;
	vec3 specularLight = textureLod(prefilteredSampler, R, lod).rgb;

	vec3 diffuse = diffuseLight * frag.albedo;
	vec3 specular = specularLight * (frag.f0 * brdf.x + brdf.y);

	vec3 iblContribution = diffuse + specular;

	outColor =  vec4(iblContribution, 1.0f);
}

