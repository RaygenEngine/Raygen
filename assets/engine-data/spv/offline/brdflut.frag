#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "hammersley.glsl"
#include "bsdf.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

#define NUM_SAMPLES 1024u
void main() { 

	float NoV = uv.s;
	float roughness = uv.t;

	// Normal always points along z-axis for the 2D lookup (shading space, no need for onb here)
	const vec3 N = vec3(0.0, 0.0, 1.0);
	vec3 V = vec3(sqrt(1.0 - NoV*NoV), 0.0, NoV);

	vec2 LUT = vec2(0.0);
	for(uint i = 0u; i < NUM_SAMPLES; i++) {
		vec2 Xi = hammersley(i, NUM_SAMPLES);
		vec3 H = importanceSampleGGX(Xi, roughness);
		vec3 L = 2.0 * dot(V, H) * H - V;

		float NoL = saturate(dot(N, L));
		float NoV = saturate(dot(N, V));
		float VoH = saturate(dot(V, H)); 
		float NoH = saturate(dot(H, N));

		if (NoL > 0.0) {
			float G = G_SchlicksmithGGX(NoL, NoV, roughness);
			float G_Vis = (G * VoH) / ((NoH * NoV) + 1e-5);
			float Fc = pow(1.0 - VoH, 5.0);
			LUT += vec2((1.0 - Fc) * G_Vis, Fc * G_Vis);
		}
	}
		outColor = vec4(LUT / float(NUM_SAMPLES), 0.0, 1.0);
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
                                                                                                                                    
                                                                                                                                              
                                                                                                                                               
                                                                                                                                        
                                                                                                                                         
                                                                                                                                          
                                                                                                                                           
                                                                                                                                             
                                                                                                                                             
                                                                                                                                                
                                                                                                                                                 
                                                                                                                                                   
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                     
                                                                                                                                                      
                                                                                                                                                       
                                                                                                                                                        
                                                                                                                                                             
                                                                                                                                                              

