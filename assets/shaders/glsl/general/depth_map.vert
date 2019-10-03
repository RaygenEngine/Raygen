#version 460 core

layout (location = 0) in vec3 pos;

out vec4 world_pos;

uniform mat4 mvp;
uniform mat4 m;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
	world_pos = m * vec4(pos, 1.0);
}  