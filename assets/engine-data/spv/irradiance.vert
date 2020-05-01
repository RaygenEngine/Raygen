#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout (location = 0) out vec3 localPos;

// in

layout (location = 0) in vec3 pos;

// uniforms

layout(push_constant) uniform ViewProj {
	mat4 v;
    mat4 p;
} push;

void main() 
{
    localPos = pos;

    mat4 rotView = mat4(mat3(push.v)); // remove translation from the view matrix
    vec4 clipPos = push.p * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos.xyzw;

}                          
                           
                             
