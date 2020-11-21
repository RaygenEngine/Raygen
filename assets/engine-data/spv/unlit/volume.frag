#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec4 color;

// uniform


void main()
{
	outColor = color;
}                               


















