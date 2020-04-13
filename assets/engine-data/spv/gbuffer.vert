#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout(location=0) out Data
{ 
	vec3 fragPos; 
	vec2 uv;
	mat3 TBN;
};

// in

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 textCoord;

// uniforms

layout(push_constant) uniform ModelData {
	mat4 modelMat;
	mat4 normalMat;
} push;

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 viewProj;
} camera;

void main() {
	gl_Position = camera.viewProj * push.modelMat * vec4(position, 1.0);

	fragPos = vec3(push.modelMat * vec4(position, 1.0));
	uv = textCoord;


	vec3 T = normalize(mat3(push.normalMat) * tangent);
   	vec3 B = normalize(mat3(push.normalMat) * bitangent);
   	vec3 N = normalize(mat3(push.normalMat)  * normal);
	
    TBN = mat3(T, B, N);
}                                       
                                        
                                         
                                            
                                             
                                              
                                               
                                                 
                                                  
                                                    
                                                     
                                                     
                                                         
                                                          
                                                            
                                                            
