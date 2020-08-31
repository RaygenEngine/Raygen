#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

#include "bsdf.h"
#include "fragment.h"
#include "shadow-sampling.h"

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
		vec3 direction;
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
		
	// spot light
	vec3 N = frag.normal; 
	vec3 V = normalize(cam.position - frag.position);
	vec3 L = normalize(-light.direction); 

	float NoL = saturate(dot(N, L));
		
	float shadow = ShadowCalculation(shadowmap, light.viewProj, frag.position, light.maxShadowBias, NoL, light.samples, light.sampleInvSpread);
	//float shadow = ShadowCalculationFast(shadowmap, light.viewProj, frag.position, light.maxShadowBias);
	vec3 Li = (1.0 - shadow) * light.color * light.intensity;       

	vec3 H = normalize(V + L);   

    float NoV = abs(dot(N, V)) + 1e-5; 
    float NoH = saturate(dot(N, H));
    float LoH = saturate(dot(L, H));

	// to get final diffuse and specular both those terms are multiplied by Li * NoL
	vec3 brdf_d = Fd_Burley(NoV, NoL, LoH, frag.diffuseColor, frag.a);
	vec3 brdf_r = Fr_CookTorranceGGX(NoV, NoL, NoH, LoH, frag.f0, frag.a);

	// so to simplify (faster math)
	vec3 finalContribution = (brdf_d + brdf_r) * Li * NoL;

    outColor = vec4(finalContribution, 1); 
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
                                                                                                                                    
                                                                                                                                              
                                                                                                                                               
                                                                                                                                        
                                                                                                                                         
                                                                                                                                          
                                                                                                                                           
                                                                                                                                             
                                                                                                                                             
                                                                                                                                                
                                                                                                                                                 
                                                                                                                                                   
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                     
                                                                                                                                                      
                                                                                                                                                       
                                                                                                                                                        
                                                                                                                                                             
                                                                                                                                                              















