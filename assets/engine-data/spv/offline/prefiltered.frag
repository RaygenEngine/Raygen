#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

#include "bsdfs.glsl"
#include "hammersley.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec3 localPos; 

// uniforms

layout(set = 0, binding = 0) uniform samplerCube skyboxSampler;

layout(push_constant) uniform PC {
	mat4 rotVp;
    float a; // roughness * roughness?
} push;

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

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
        vec3 H  = ImportanceSampleGGX(Xi, N, push.a);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NoL = saturate(dot(N, L));

        if(NoL > 0.0)
        {
            float NoH = saturate(dot(N, H));        
            float HoV = saturate(dot(H, V)); 

            float D = D_GGX(NoH, push.a); 
            float pdf = (D * NoH / (4 * HoV)) + 0.0001;
             
            float saTexel = 4.0 * PI / (6.0f * push.a);
            float saSample = 1.0 / float(SAMPLE_COUNT * pdf + 0.00001);
             
            float mipLevel = push.a == 0.0 ? 0.0 :  0.5 * log2(saSample / saTexel);
                                 
            prefilteredColor += textureLod(skyboxSampler, L, mipLevel).rgb * NoL;     
            totalWeight += NoL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    outColor = vec4(prefilteredColor, 1.0);
}                                                                                                                          
                                                                                                                                                                                    
                                                 
                                                  
                                                   
                                                    
                                                     
                                                       
                                                        
                                                         
                                                         
                                                          
                                                           
                                                            
                                                             
                                                              
                                                               
                                                                 
                                                                  
                                                                   
                                                                    
                                                                     
                                                                      
