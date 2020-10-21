#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "attachments.glsl"
#include "fragment.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

void main() {

	float a = texture(g_SpecularSampler, uv).a;
	a = a * a;
	if (a >= 0.001)
		outColor =  texture(rasterDirectSampler, uv);
	else
	 	outColor = texture(rtIndirectSampler, uv); 

}                               
