#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D albedoSampler;

void main() {
    vec3 hdrColor = texture(albedoSampler, uv).rgb;
  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.5);
    // Gamma correction 
	mapped = pow(mapped, vec3(1.0 / 2.2f));

    outColor = vec4(mapped, 1.0f);
}