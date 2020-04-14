#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive: enable

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

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

layout(set = 2, binding = 0) uniform UBO_Ambient {
    vec3 color;
	float pad0;
} ambient;

layout(set = 2, binding = 1) uniform samplerCube skyboxSampler;

vec3 ReconstructWCS(vec2 uvc)
{
	vec4 clipPos; // clip space reconstruction
	clipPos.x = uv.x;
	clipPos.y = -uv.y;
	clipPos.z = 1.0;
	clipPos.w = 1.0;
	
	vec4 pwcs = camera.viewProjInv * clipPos; // clip space -> world space

	return pwcs.xyz / pwcs.w; // return world space pos xyz
}

void main() {

	float currentDepth = texture(depthSampler, uv).r;

	outColor = vec4(0.0, 0.0, 0.0, 1.0);

	if(currentDepth == 1.0)
	{
		vec3 dir = normalize(ReconstructWCS(uv) - camera.position);
		//dir.y = -dir.y;
		
		outColor = texture(skyboxSampler, dir);
		//return;
	}

	vec3 emissive = texture(emissiveSampler, uv).rgb;
	vec4 specular = texture(specularSampler, uv);
	
	vec3 color = emissive;
	color = mix(color, color * specular.b, specular.a);
	
	outColor += vec4(color, 1);
}                               
                                
                                 
                                  
                                   
                                    
                                     
                                            
                                               
                                                 
                                                  
                                                   
                                                                                                                                    
                                                                                                                                              
                                                                                                                                               
                                                                                                                                        
                                                                                                                                         
                                                                                                                                          
                                                                                                                                           
                                                                                                                                             
                                                                                                                                             
                                                                                                                                                
                                                                                                                                                 
                                                                                                                                                   
                                                                                          
                                                                                           
                                                                                            
                                                                                                
                                                                                               
                                                                                                 
                                                                                                 
                                                                                                   
                                                                                                    
                                                                                                    
                                                                                                     
                                                                                                           
