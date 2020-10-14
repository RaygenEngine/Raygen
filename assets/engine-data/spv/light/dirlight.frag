#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "bsdf.glsl"
#include "fragment.glsl"
#include "shadow.glsl"
#include "sampling.glsl"
#include "onb.glsl"
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

layout(set = 2, binding = 0) uniform UBO_Dirlight {
		vec3 front;
		float pad0;

		mat4 viewProj;
		vec3 color;
		float pad3;

		float intensity;

		float maxShadowBias;
		int samples;
		float sampleInvSpread; 
} light;

layout(set = 3, binding = 0) uniform sampler2DShadow shadowmap;

void main() {

	float depth = texture(g_DepthSampler, uv).r;

	if(depth == 1.0) {
		discard;
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

	Onb shadingOrthoBasis = branchlessOnb(frag.normal);

	vec3 V = normalize(cam.position - frag.position);
	vec3 L = normalize(-light.front); // explicit light dir

	toOnbSpace(shadingOrthoBasis, V);
	toOnbSpace(shadingOrthoBasis, L);

	vec3 H = normalize(V + L); 
	float NoL = max(Ndot(L), BIAS);
	float NoV = max(Ndot(V), BIAS);
	float NoH = max(Ndot(H), BIAS); 
	float LoH = max(dot(L, H), BIAS);

	float shadow = ShadowCalculation(shadowmap, light.viewProj, frag.position, light.maxShadowBias, NoL, light.samples, light.sampleInvSpread);

	vec3 Li = (1.0 - shadow) * light.color * light.intensity;     

	// Li comes from direct light path
	vec3 finalContribution = DirectLightBRDF(NoL, NoV, NoH, LoH, frag.a, frag.albedo, frag.f0) * Li * NoL;

	outColor = vec4(finalContribution, 1);
}                               
                                
                                 


