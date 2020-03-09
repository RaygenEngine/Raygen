#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
// rgb: albedo, a: opacity
layout (location = 2) out vec4 gAlbedoOpacity;
// r: metallic, g: roughness, b: occlusion, a: occlusion strength
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;

in Data
{ 
	vec3 fragPos; 
	vec2 uv[2];
	mat3 TBN;
} dataIn;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    vec3 hdrColor = texture(texSampler, fragTexCoord).rgb;
  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.5);
    // Gamma correction 
	mapped = pow(mapped, vec3(1.0 / 2.2f));

    outColor = vec4(mapped, 1.0f);
}