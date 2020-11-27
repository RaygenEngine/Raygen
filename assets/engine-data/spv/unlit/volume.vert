#version 460

// out

layout(location = 0) out vec4 color;

// in

layout(location = 0) in vec3 position;

// uniforms

layout(push_constant) uniform PC {
	mat4 volumeMatVp;
    vec4 pccolor;
};


void main() 
{
    gl_Position = volumeMatVp * vec4(position, 1.0);
    color = pccolor;
}
