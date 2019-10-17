#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 4) in vec2 textCoord0;
layout (location = 5) in vec2 textCoord1;

uniform mat4 mvp;

out Data
{ 
	vec2 textCoord[2];
} dataOut;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
	dataOut.textCoord[0] = textCoord0; 
	dataOut.textCoord[1] = textCoord1; 
}  