#version 460
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

// out

// in

layout(location=0) in vec2 uv;

// uniforms

layout(set = 1, binding = 0) uniform UBO_Material { GltfMaterial mat; };
layout(set = 1, binding = 1) uniform sampler2D baseColorSampler;

void main() {
	vec4 sampledBaseColor = texture(baseColorSampler, uv);

	float opacity = sampledBaseColor.a * mat.baseColorFactor.a;

	if(mat.alphaMode == ALPHA_MODE_MASK && opacity <= mat.alphaCutoff) {
		discard;
	}
}                                                                                        

