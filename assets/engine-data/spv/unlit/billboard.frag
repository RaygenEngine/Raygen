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
	float a = texture(f, uv).a;

	const float thres = 0.05;
	if(a < thres){
		discard;
	}
	
	outColor = a < 1 - thres ? vec4(0) : vec4(1);
}                               





