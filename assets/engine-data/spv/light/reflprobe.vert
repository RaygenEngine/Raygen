#version 460

// out

// in

layout(location = 0) in vec3 position;

// uniforms

layout(push_constant) uniform PC {
	mat4 reflVolMatVP;
    vec3 reflPosition;
} push;


void main() 
{
    gl_Position = push.reflVolMatVP * vec4(position, 1.0);
}                
                
