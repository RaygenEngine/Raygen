#version 460

// out

layout(location = 0) out vec2 uv;

// in

layout(location = 0) in vec3 position;

// uniforms

layout(push_constant) uniform PC {
	mat4 lightVolMatVP;
} push;


void main() 
{
    gl_Position = push.lightVolMatVP * vec4(position, 1.0);
    uv = ((gl_Position.xy / gl_Position.w) + 1.f) / 2.f;
}                
                
