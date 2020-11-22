#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 0, binding = 0) uniform sampler2D f;

void main()
{
	outColor = vec4(texture(f, uv).a);
}                               



