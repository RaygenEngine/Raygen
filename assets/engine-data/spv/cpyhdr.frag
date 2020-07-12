#version 450 
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform 

layout(set = 0, binding = 0) uniform sampler2D hdrColorSampler;

void main( ) {

	vec3 hdrColor = texture(hdrColorSampler, uv).rgb;

	// gamma correction / exposure
	const float gamma = 2.0;
	const float exposure = 2.5;

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

	outColor = vec4(mapped, 1.0);
}                                                                                                                          
                                                                                                                                       
                                                                                                                                                                 
                                                                                                                       
                                                                                                                     
                                                                                                                      
                                                                                                                        
                                                                                                                         
                                                                                                                                       
                                                                                                                            
                                                                                                                             
                                                                                                                              
                                                                                                                               
                                                                                                                                                
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                    
                                                                                                                                                     

