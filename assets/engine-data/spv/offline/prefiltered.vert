#version 460

// out

layout (location = 0) out vec3 localPos;

// in

layout (location = 0) in vec3 pos;

// uniforms

layout(push_constant) uniform PC {
	mat4 rotVp;
    float a;
    float skyboxRes;
} push;

void main() 
{
    localPos = pos;

    vec4 clipPos = push.rotVp * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}                          
                           
                             
                             
