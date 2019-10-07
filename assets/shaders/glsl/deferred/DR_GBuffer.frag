#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoOpacity;
layout (location = 3) out vec4 gMetallicRoughnessOcclusionOcclusionStrength;
layout (location = 4) out vec4 gEmissive;
  
in Data
{ 
	vec3 world_pos;  

	vec2 text_coord[2];
	
	mat3 TBN;
} dataIn;

uniform vec4 base_color_factor;
uniform vec3 emissive_factor;
uniform float metallic_factor;
uniform float roughness_factor;
uniform float normal_scale;
uniform float occlusion_strength;

uniform int base_color_texcoord_index;
uniform int metallic_roughness_texcoord_index;
uniform int emissive_texcoord_index;
uniform int normal_texcoord_index;
uniform int occlusion_texcoord_index;

layout(binding=0) uniform sampler2D baseColorSampler;
layout(binding=1) uniform sampler2D metallicRoughnessSampler;
layout(binding=2) uniform sampler2D emissiveSampler;
layout(binding=3) uniform sampler2D normalSampler;
layout(binding=4) uniform sampler2D occlusionSampler;

void main()
{
	// sample material textures
	vec4 sampled_base_color = texture(baseColorSampler, dataIn.text_coord[base_color_texcoord_index]);
	vec4 sampled_metallic_roughness = texture(metallicRoughnessSampler, dataIn.text_coord[metallic_roughness_texcoord_index]);
	vec4 sampled_emissive = texture(emissiveSampler, dataIn.text_coord[emissive_texcoord_index]);
	vec4 sampled_sample_normal = texture(normalSampler, dataIn.text_coord[normal_texcoord_index]);
	vec4 sampled_occlusion = texture(occlusionSampler, dataIn.text_coord[occlusion_texcoord_index]);
	
	// final material values
	
	// rgb : albedo, a: opacity
	vec4 albedo = sampled_base_color * base_color_factor;
	float metallic = sampled_metallic_roughness.b * metallic_factor;
	float roughness = sampled_metallic_roughness.g * roughness_factor;
	float occlusion = sampled_occlusion.r;
	vec3 emissive = sampled_emissive.rgb * emissive_factor;
	vec3 normal = sampled_sample_normal.rgb;

    // position
    gPosition = dataIn.world_pos;
	
    // normal (with normal mapping)
	vec3 N = normalize((normal * 2.0 - 1.0) * vec3(normal_scale, normal_scale, 1.0));
	N = normalize(dataIn.TBN * N);
    gNormal = normalize(N);
	
    // albedo opacity
    gAlbedoOpacity = albedo;
	
	// spec params
	gMetallicRoughnessOcclusionOcclusionStrength = vec4(metallic, roughness, occlusion, occlusion_strength);
	
	// emissive
	gEmissive = vec4(emissive, 1.f);
}