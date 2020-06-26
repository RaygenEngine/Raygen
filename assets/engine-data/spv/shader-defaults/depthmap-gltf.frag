#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

// in

layout(location=0) in vec2 uv;

// uniforms

layout(set = 0, binding = 0) uniform UBO_Material {
	// factors
    vec4 baseColorFactor;
	vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// alpha mask
	float alphaCutoff;
	int mask;
} material;

layout(set = 0, binding = 1) uniform sampler2D baseColorSampler;
layout(set = 0, binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(set = 0, binding = 3) uniform sampler2D occlusionSampler;
layout(set = 0, binding = 4) uniform sampler2D normalSampler;
layout(set = 0, binding = 5) uniform sampler2D emissiveSampler;

void main() {
	vec4 sampledBaseColor = texture(baseColorSampler, uv);

	float opacity = sampledBaseColor.a * material.baseColorFactor.a;

	// mask mode and cutoff
	if(material.mask == 1 && opacity < material.alphaCutoff)
		discard;
}                                                                                        

