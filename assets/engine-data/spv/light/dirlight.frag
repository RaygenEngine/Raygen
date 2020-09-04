#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

#include "bsdf.h"
#include "fragment.h"
#include "shadow.h"
#include "sampling.h"
#include "onb.h"
#include "attachments.h"

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

layout(set = 2, binding = 0) uniform UBO_Directionallight {
		vec3 front;
		float pad0;

		// CHECK: could pass this mat from push constants (is it better tho?)
		// Lightmap
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

	if(depth == 1.0)
	{
		discard;
	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		g_NormalSampler,
		g_ColorSampler,
		g_MRROSampler,
		g_EmissiveSampler,
		uv);

	Onb shadingOrthoBasis = branchlessOnb(frag.normal);

	vec3 V = normalize(cam.position - frag.position);
	vec3 L = normalize(-light.front); // explicit light dir

	toOnbSpace(shadingOrthoBasis, V);
	toOnbSpace(shadingOrthoBasis, L);

	float NoL = Ndot(L);
	// TODO: missing geometric / face normal tests
	outColor = vec4(vec3(0), 1);
	if(NoL > 0) // test abs, saturate on edge cases (remove branching)
	{
		float shadow = ShadowCalculation(shadowmap, light.viewProj, frag.position, light.maxShadowBias, NoL, light.samples, light.sampleInvSpread);

		vec3 Li = (1.0 - shadow) * light.color * light.intensity;     

		vec3 H = normalize(V + L);
		float NoV = Ndot(V);
		float NoH = Ndot(H);
		float LoH = dot(L, H);

		vec3 brdf_d = DisneyDiffuse(NoL, NoV, LoH, frag.a, frag.diffuseColor);
		vec3 brdf_r = SpecularTerm(NoL, NoV, NoH, LoH, frag.a, frag.f0);

		// Li comes from direct light path
		vec3 finalContribution = (brdf_d + brdf_r) * Li * NoL;

		outColor = vec4(finalContribution, 1);
	}
}                               
                                
                                 
                                  