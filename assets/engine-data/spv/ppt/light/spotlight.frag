#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

#include "bsdf.h"
#include "fragment.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 0, binding = 0) uniform sampler2D normalsSampler;
layout(set = 0, binding = 1) uniform sampler2D baseColorSampler;
layout(set = 0, binding = 2) uniform sampler2D surfaceSampler;
layout(set = 0, binding = 3) uniform sampler2D emissiveSampler;
layout(set = 0, binding = 4) uniform sampler2D depthSampler;

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
		vec3 direction;
		float pad1;

		// CHECK: could pass this mat from push constants (is it better tho?)
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
} light;

layout(set = 3, binding = 0) uniform sampler2D shadowmap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
   // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords ;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowmap, projCoords.xy * 0.5 + 0.5).r; 
    // get depth of current frag from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - closestDepth > 0.005  ? 1.0 : 0.0;

    return shadow;
}  

void main() {

	float depth = texture(depthSampler, uv).r;

	if(depth == 1.0)
	{
		discard;
	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		normalsSampler,
		baseColorSampler,
		surfaceSampler,
		emissiveSampler,
		uv);
		
	// spot light
	vec3 N = frag.normal;
	vec3 V = normalize(cam.position - frag.position);
	vec3 L = normalize(light.position - frag.position); 
	
	// attenuation
	float dist = length(light.position - frag.position);
	float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
  			     light.quadraticTerm * (dist * dist));
	
    // spot effect (soft edges)
	float theta = dot(L, -light.direction);
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	vec4 lightSpacePos = light.viewProj * vec4(frag.position,1.0);
	float shadow = ShadowCalculation(lightSpacePos);
		//return; 
	vec3 Li = (1.0 - shadow) * light.color * light.intensity * attenuation * spotEffect; 

	vec3 diffuseColor = (1.0 - frag.metallic) * frag.baseColor;
	vec3 f0 = 0.16 * frag.reflectance * frag.reflectance * (1.0 - frag.metallic) + frag.baseColor * frag.metallic;
	float a = frag.roughness * frag.roughness;

	vec3 Lo = BRDF(V, L, N, diffuseColor, f0, a) * Li * saturate(dot(N, L));

    outColor = vec4(Lo, 1);
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
                                                                                                                                    
                                                                                                                                              
                                                                                                                                               
                                                                                                                                        
                                                                                                                                         
                                                                                                                                          
                                                                                                                                           
                                                                                                                                             
                                                                                                                                             
                                                                                                                                                
                                                                                                                                                 
                                                                                                                                                   
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                     
                                                                                                                                                      
                                                                                                                                                       
                                                                                                                                                        
                                                                                                                                                             
                                                                                                                                                              
