#version 460
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

// out

// in

layout(location=0) in vec2 uv;

// uniforms

layout(set = 0, binding = 0) uniform UBO_Material { GltfMaterial mat; };
layout(set = 0, binding = 1) uniform sampler2D baseColorSampler;

void main() {
	vec4 sampledBaseColor = texture(baseColorSampler, uv);

	float opacity = sampledBaseColor.a * mat.baseColorFactor.a;

	// mask mode and cutoff
	if(mat.mask == 1 && opacity < mat.alphaCutoff)
		discard;
}                                                                                        

