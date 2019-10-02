#version 460 core

layout (location = 0) in vec3 pos;

uniform mat4 light_space_matrix;
uniform mat4 m;

void main()
{
    gl_Position = light_space_matrix * m * vec4(pos, 1.0);
}  