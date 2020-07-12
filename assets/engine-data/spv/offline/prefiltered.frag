#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

#include "bsdf.h"
#include "hammersley.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec3 localPos; 

// uniforms

layout(set = 0, binding = 0) uniform samplerCube skyboxSampler;

layout(push_constant) uniform PC {
	mat4 rotVp;
    float a; // a = roughness * roughness
    float skyboxRes;
} push;

void main( ) {
	vec3 N = normalize(localPos);    
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);     
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H  = importanceSampleGGX(Xi, push.a, N);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);

        if(NdotL > 0.0)
        {
            float NdotH =  max(dot(N, H), 0.0);        
            float HdotV = max(dot(H, V), 0.0); 

            float D = D_GGX(NdotH, push.a); 
            float pdf = (D * NdotH / (4 * HdotV)) + 0.0001;
             
            float saTexel = 4.0 * PI / (6.0f * push.a);
            float saSample = 1.0 / float(SAMPLE_COUNT * pdf + 0.00001);
             
            float mipLevel = push.a == 0.0 ? 0.0 :  0.5 * log2(saSample / saTexel);
                                 
            prefilteredColor += textureLod(skyboxSampler, L, mipLevel).rgb * NdotL;     
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    outColor = vec4(prefilteredColor, 1.0);
}                                                                                                                          
                                                                                                                                                                                    
                                                 
                                                  
                                                   
                                                    
                                                     
                                                       
                                                        
                                                         
                                                         
                                                          
                                                           
                                                            
                                                             
                                                              
                                                               
                                                                 
                                                                  
                                                                   
                                                                    
                                                                     
                                                                      
