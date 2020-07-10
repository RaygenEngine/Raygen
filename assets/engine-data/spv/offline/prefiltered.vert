#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout (location = 0) out vec3 localPos;

// in

layout (location = 0) in vec3 pos;

// uniforms

layout(push_constant) uniform PC {
	mat4 rotVp;
    float roughness;
    float skyboxRes;
} push;

void main() 
{
    localPos = pos;

    vec4 clipPos = push.rotVp * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}                          
                           
                             
                             
