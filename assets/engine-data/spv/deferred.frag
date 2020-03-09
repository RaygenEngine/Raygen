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
    vec3 hdrColor = texture(depthSampler, uv).rgb;
  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.5);
    // Gamma correction 
	mapped = pow(mapped, vec3(1.0 / 2.2f));

    outColor = vec4(mapped, 1.0f);
}