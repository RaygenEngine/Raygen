#version 450 
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

#include "tonemapping.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform 

layout(set = 0, binding = 0) uniform sampler2D hdrColorSampler;

// TODO:
// push constant tonemapper, gamma, exposure

void main( ) {

	vec3 hdrColor = texture(hdrColorSampler, uv).rgb;

	// gamma correction / exposure
	const float gamma = 2.2;
	const float exposure = 2.5;
	
	// TONEMAP_DEFAULT
	// TONEMAP_UNCHARTED
	// TONEMAP_HEJLRICHARD
	// TONEMAP_ACES

	outColor = vec4(tonemap(hdrColor, TONEMAP_ACES, gamma, exposure), 1.0);
}                                                                                                                          
                                                                                                                                       
                                                                                                                                                                 
                                                                                                                       
                                                                                                                     
                                                                                                                      
                                                                                                                        
                                                                                                                         
                                                                                                                                       
                                                                                                                            
                                                                                                                             
                                                                                                                              
                                                                                                                               
                                                                                                                                                
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                    
                                                                                                                                                     


