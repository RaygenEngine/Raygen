#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.h"
#include "fragment.h"


// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

void main() {

	vec3 direct = texture(rasterDirectSampler, uv).rgb;
	vec3 indirect = texture(rtIndirectSampler, uv).rgb;

    outColor = vec4(direct + indirect, 1.f); 
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
                                                                                                                                    
                                                                                                                                              
                                                                                                                                               
                                                                                                                                        
                                                                                                                                         
                                                                                                                                          
                                                                                                                                           
                                                                                                                                             
                                                                                                                                             
                                                                                                                                                
                                                                                                                                                 
                                                                                                                                                   
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                     
                                                                                                                                                      
                                                                                                                                                       
                                                                                                                                                        
                                                                                                                                                             
                                                                                                                                                              














