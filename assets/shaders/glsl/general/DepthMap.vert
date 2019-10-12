#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 4) in vec2 text_coord0;
layout (location = 5) in vec2 text_coord1;

uniform mat4 mvp;

out Data
{ 
	vec2 text_coord[2];
} dataOut;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
	dataOut.text_coord[0] = text_coord0; 
	dataOut.text_coord[1] = text_coord1; 
}  