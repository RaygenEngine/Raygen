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
	if (texture(f, uv).a < 0.15) {
		discard;
	}
	if (texture(f, uv).a < 0.9) {
		outColor = vec4(0);
	}
	else {
		outColor = vec4(1); //vec4(texture(f, uv).a);
	}
}
