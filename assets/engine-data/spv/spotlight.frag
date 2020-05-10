#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive: enable

#include "microfacet_bsdf.h"
#include "fragment.h"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 0, binding = 0) uniform sampler2D positionsSampler;
layout(set = 0, binding = 1) uniform sampler2D normalsSampler;
layout(set = 0, binding = 2) uniform sampler2D albedoOpacitySampler;
layout(set = 0, binding = 3) uniform sampler2D specularSampler;
layout(set = 0, binding = 4) uniform sampler2D emissiveSampler;
layout(set = 0, binding = 5) uniform sampler2D depthSampler;

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} camera;

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
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - closestDepth > 0.005  ? 1.0 : 0.0;

    return shadow;
}  

void main() {

	// PERF:
	Fragment fragment = GetFragmentFromGBuffer(
		positionsSampler,
		normalsSampler,
		albedoOpacitySampler,
		specularSampler,
		emissiveSampler,
		depthSampler,
		uv);

	if(fragment.depth == 1.0)
	{
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	// spot light
	vec3 N = fragment.normal;
	vec3 V = normalize(camera.position - fragment.position);
	vec3 L = normalize(light.position - fragment.position); 
	
	// attenuation
	float dist = length(light.position - fragment.position);
	float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
  			     light.quadraticTerm * (dist * dist));
	
    // spot effect (soft edges)
	float theta = dot(L, -light.direction);
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	vec4 lightSpacePos = light.viewProj * vec4(fragment.position,1.0);
	float shadow = ShadowCalculation(lightSpacePos);
		//return; 
	vec3 Li = (1.0 - shadow) * light.color * light.intensity * attenuation * spotEffect; 

	vec3 Lo = CookTorranceMicrofacetBRDF_GGX(L, V, N, fragment.albedo, fragment.metallic, fragment.roughness) * Li * max(dot(N, L), 0.0);

    // TODO: emissive at a later pass
    vec3 hdrColor = Lo;

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.2);
    // Gamma correction 
	mapped = pow(mapped, vec3(1.0 / 2.2));
  
    outColor = vec4(mapped, 1);
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
                                                                                                                                    
                                                                                                                                              
                                                                                                                                               
                                                                                                                                        
                                                                                                                                         
                                                                                                                                          
                                                                                                                                           
                                                                                                                                             
                                                                                                                                             
                                                                                                                                                
                                                                                                                                                 
                                                                                                                                                   
                                                                                                                                                  
                                                                                                                                                   
                                                                                                                                                     
                                                                                                                                                      
                                                                                                                                                       
                                                                                                                                                        
                                                                                                                                                             
                                                                                                                                                              
