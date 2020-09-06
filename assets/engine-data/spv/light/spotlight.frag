#version 460
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "fragment.glsl"
#include "shadow.glsl"
#include "bsdf.glsl"
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

layout(set = 2, binding = 0) uniform UBO_Spotlight {
		vec3 position;
		float pad0;
		vec3 front;
		float pad1;

		// Lightmap
		mat4 viewProj;
		vec3 color;
		float pad3;

		float intensity;

		float near;
		float far;

		float outerCutOff;
		float innerCutOff;

		float constantTerm;
		float linearTerm;
		float quadraticTerm;

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
	vec3 L = normalize(light.position - frag.position);
	vec3 lDir = -light.front;

	toOnbSpace(shadingOrthoBasis, V);
	toOnbSpace(shadingOrthoBasis, L);
	toOnbSpace(shadingOrthoBasis, lDir);  
	
	// attenuation
	float dist = length(light.position - frag.position);
	float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
  			     light.quadraticTerm * (dist * dist));
	
    // spot effect (soft edges)
	float theta = dot(L, lDir);
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	vec3 H = normalize(V + L); 
	float NoL = max(Ndot(L), BIAS);
	float NoV = max(Ndot(V), BIAS);
	float NoH = max(Ndot(H), BIAS); 
	float LoH = max(dot(L, H), BIAS);

	float shadow = ShadowCalculation(shadowmap, light.viewProj, frag.position, 
	light.maxShadowBias, NoL, light.samples, light.sampleInvSpread);

	vec3 Li = (1.0 - shadow) * light.color * light.intensity * attenuation * spotEffect; 

	vec3 brdf_d = LambertianDiffuse(frag.diffuseColor);
	vec3 brdf_r = SpecularTerm(NoL, NoV, NoH, LoH, frag.a, frag.f0);

	//vec3 kd = vec3(1) - F_Schlick(LoH, frag.f0) ;

	// Li comes from direct light path
	vec3 finalContribution = (brdf_d + brdf_r) * Li * NoL;

	outColor = vec4(finalContribution, 1);
}                               


















