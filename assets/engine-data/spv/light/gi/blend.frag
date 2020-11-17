#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout (input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput indirectInput;
layout (input_attachment_index = 1, set = 0, binding = 0) uniform subpassInput aoInput;



void main()
{

	float occlusion = subpassLoad(aoInput).r;
	vec3 indirectLight = subpassLoad(indirectInput).rgb;
	
	outColor = vec4(occlusion * indirectLight, 1);
}                               
           





















