#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec3 localPos; 

// uniforms

layout(set = 0, binding = 0) uniform samplerCube skyboxSampler;

void main( ) {
    // the sample direction equals the hemisphere's orientation 
    vec3 normal = normalize(localPos); // dir to cubemap texel pos = actual sample dir
  
    vec3 irradiance = vec3(0.0);
  
	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up         = cross(normal, right);
	
	float sampleDelta = 0.025;
	float nrSamples = 0.0; 
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

			irradiance += texture(skyboxSampler, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));
  
	outColor = vec4(irradiance, 1.f);
}                                                                                                                          
                                                                                                                                                                                    
                                                 
                                                  
                                                   
                                                    
                                                     
                                                       
                                                        
                                                         
                                                         
                                                          
                                                           
                                                            
                                                             
                                                              
                                                               
                                                                 
                                                                  
                                                                   
                                                                    
                                                                     
                                                                      
