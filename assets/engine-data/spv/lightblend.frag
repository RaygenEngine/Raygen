#version 460
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

void main() {

	outColor = vec4(texture(rtIndirectSampler, uv).rgb +  texture(rasterDirectSampler, uv).rgb, 1.0);
}                               
