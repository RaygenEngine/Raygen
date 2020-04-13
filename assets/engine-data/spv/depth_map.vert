#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout(location = 0) out vec2 uv;

// in

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textCoord;

// uniforms

layout(push_constant) uniform ModelData {
	mat4 mvp;
} push;

void main() {
    gl_Position = push.mvp * vec4(position, 1.0);
	uv = textCoord;
}                                       
                                        
                                         
                                            
                                             
                                              
                                               
                                                 
                                                  
                                                    
                                                     
                                                     
                                 
                                   
                                   
