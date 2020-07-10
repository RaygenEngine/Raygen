#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive: enable

#include "microfacet_bsdf.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

// Sascha


#define NUM_SAMPLES 1024u
vec2 LUT_BRDF(float NoV, float roughness)
{
	// Normal always points along z-axis for the 2D lookup 
	const vec3 N = vec3(0.0, 0.0, 1.0);
	vec3 V = vec3(sqrt(1.0 - NoV*NoV), 0.0, NoV);

	vec2 LUT = vec2(0.0);
	for(uint i = 0u; i < NUM_SAMPLES; i++) {
		vec2 Xi = hammersley2d(i, NUM_SAMPLES);
		vec3 H = importanceSample_GGX(Xi, roughness, N);
		vec3 L = 2.0 * dot(V, H) * H - V;

		float dotNL = max(dot(N, L), 0.0);
		float dotNV = max(dot(N, V), 0.0);
		float dotVH = max(dot(V, H), 0.0); 
		float dotNH = max(dot(H, N), 0.0);

		if (dotNL > 0.0) {
			float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
			float G_Vis = (G * dotVH) / (dotNH * dotNV);
			float Fc = pow(1.0 - dotVH, 5.0);
			LUT += vec2((1.0 - Fc) * G_Vis, Fc * G_Vis);
		}
	}
	return LUT / float(NUM_SAMPLES);
}

void main() { 
	outColor = vec4(LUT_BRDF(uv.s, uv.t), 0.0, 1.0);
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
                                                                                                                                    
                                                                                                                                              
                                                                                                                                               
                                                                                                                                        
                                                                                                                                         
                                                                                                                                          
                                                                                                                                           
                                                                                                                                             
                                                                                                                                             
                                                                                                                                                
                                                                                                                                                 
                                                                                                                                                   
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                     
                                                                                                                                                      
                                                                                                                                                       
                                                                                                                                                        
                                                                                                                                                             
                                                                                                                                                              

