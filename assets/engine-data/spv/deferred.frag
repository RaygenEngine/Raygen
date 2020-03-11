#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(binding = 0) uniform sampler2D positionsSampler;
layout(binding = 1) uniform sampler2D normalsSampler;
layout(binding = 2) uniform sampler2D albedoOpacitySampler;
layout(binding = 3) uniform sampler2D specularSampler;
layout(binding = 4) uniform sampler2D emissiveSampler;
layout(binding = 5) uniform sampler2D depthSampler;


void main() {
    outColor = texture(albedoOpacitySampler, uv);
}