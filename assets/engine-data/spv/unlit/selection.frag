#version 460
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

// out

layout (location = 0) out vec4 outColor;

// in

// uniforms

void main() {
	outColor = vec4(0.04, 0.28, 0.26, 1.0);
}
