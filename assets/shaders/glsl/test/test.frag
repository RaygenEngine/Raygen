#version 460 core

out vec4 out_color;
  
in Data
{ 
	vec3 world_pos;  
	vec3 world_normal;
	vec4 world_tangent;
	vec3 world_bitangent;

	vec3 tangent_pos;
	vec3 tangent_view_pos;
	vec3 tangent_light_pos;
	
	vec2 text_coord0;
	vec2 text_coord1;
	
	mat3 TBN;
} dataIn;

uniform int mode;

uniform vec3 view_pos;

uniform vec3 light_pos;
uniform vec3 light_color;
uniform float light_intensity;

uniform vec3 ambient;

uniform vec4 base_color_factor;
uniform vec3 emissive_factor;
uniform float metallic_factor;
uniform float roughness_factor;
uniform float normal_scale;
uniform float occlusion_strength;
uniform int alpha_mode;
uniform float alpha_cutoff;
uniform bool double_sided;

layout(binding=0) uniform sampler2D baseColorSampler;
layout(binding=1) uniform sampler2D metallicRoughnessSampler;
layout(binding=2) uniform sampler2D emissiveSampler;
layout(binding=3) uniform sampler2D normalSampler;
layout(binding=4) uniform sampler2D occlusionSampler;

#define M_1_PIf 0.318309886183790671538f
#define M_PIf 3.14159265358979323846f

// very basic unoptimized surface test shader
void main()
{
	const vec4 base_color = texture(baseColorSampler, dataIn.text_coord0);
	const vec4 metallic_roughness = texture(metallicRoughnessSampler, dataIn.text_coord0);
	const vec4 emissive = texture(emissiveSampler, dataIn.text_coord0);
	const vec4 sample_normal = texture(normalSampler, dataIn.text_coord0);
	const vec4 occlusion = texture(occlusionSampler, dataIn.text_coord0);
	
	// missing
	out_color = vec4(1.f, 0.f, 1.f, 1.f);
	
	if(mode == 0)
	{
		// mask mode and cutoff, TODO: handle blend
		if(alpha_mode == 1 && base_color.a < alpha_cutoff)
			discard;
	
		// TODO: tangent space calcs
		vec3 scaled_normal = normalize((sample_normal.rgb * 2.0 - 1.0) * vec3(normal_scale, normal_scale, 1.0));
		scaled_normal = normalize(dataIn.TBN * scaled_normal);
		
		vec3 light_dir = normalize(light_pos - dataIn.world_pos); 
	
		float NoL = max(dot(scaled_normal, light_dir), 0.0);
		
		vec4 albedo = (base_color * base_color_factor) + vec4(ambient, 1.0);
		vec3 emission = emissive.rgb * emissive_factor;
		vec3 light = light_color * light_intensity;
		
		out_color = NoL * albedo * vec4(light, 1.0) + vec4(emission, 1.0);
		out_color = mix(out_color, out_color * occlusion.rrrr, occlusion_strength);
		
		return;
	}
	
	// TODO: different rendering code path
	switch (mode-1)
	{
		case 0: // base color map
			out_color = base_color;
			break;
			
		case 1: // base color factor
			out_color = base_color_factor;
			break;
			
		case 2: // base color final
			out_color = base_color * base_color_factor;
			break;
			
		case 3: // metallic map
			out_color = vec4(metallic_roughness.bbb, 1.0);
			break;
			
		case 4: // metallic factor
			out_color = vec4(metallic_factor);
			break;
			
		case 5: // metallic final
			out_color = vec4(metallic_roughness.bbb, 1.0) * metallic_factor;
			break;
			
		case 6: // roughness map
			out_color = vec4(metallic_roughness.ggg, 1.0);
			break;
			
		case 7: // roughness factor
			out_color = vec4(roughness_factor);
			break;
			
		case 8: // roughness final
			out_color = vec4(metallic_roughness.ggg, 1.0) * roughness_factor;
			break;
			
		case 9: // normal
			out_color = vec4(dataIn.world_normal, 1.0);
			break;
			
		case 10: // normal scale
			out_color = vec4(normal_scale);
			break;
			
		case 11: // normal map
			out_color = sample_normal;
			break;
			
		case 12: // normal final; TODO
			vec3 scaledNormal = normalize((sample_normal.rgb * 2.0 - 1.0) * vec3(normal_scale, normal_scale, 1.0));
			out_color = vec4(normalize(dataIn.TBN * scaledNormal), 1);
			break;
			
		case 13: // tangent
			out_color = dataIn.world_tangent;
			break;
			
		case 14: // tangent handedness
			out_color = vec4(dataIn.world_tangent.a);
			break;
			
		case 15: // bitangent
			out_color = vec4(dataIn.world_bitangent, 1.0);
			break;
			
		case 16: // occlusion map
			out_color = vec4(occlusion.rrr, 1.0);
			break;
			
		case 17: // occlusion strength
			out_color = vec4(occlusion_strength);
			break;
			
		case 18: // occlusion final
			out_color = mix(base_color, base_color * occlusion.rrrr, occlusion_strength);
			break;
			
		case 19: // emissive map
			out_color = vec4(emissive.rgb, 1.0);
			break;
			
		case 20: // emissive factor
			out_color = vec4(emissive_factor, 1.0);
			break;
			
		case 21: // emissive final
			out_color = vec4(emissive.rgb*emissive_factor, 1.0);
			break;
			
		case 22: // opacity map
			out_color = vec4(base_color.a);
			break;
			
		case 23: // opacity factor
			out_color = vec4(base_color_factor.a);
			break;
			
		case 24: // opacity final
			out_color = base_color;
			if(base_color.a < alpha_cutoff)
				discard;
			break;
			
		case 25: // texture coordinate 0
			out_color = vec4(dataIn.text_coord0, 0.0, 1.0);
			break;
			
		case 26: // texture coordinate 1
			out_color = vec4(dataIn.text_coord1, 0.0, 1.0);
			break;
			
		case 27: // alpha mode
			switch (alpha_mode)
			{
			// opaque
			case 0:
				out_color = vec4(1.f, 0.f, 0.f, 1.f);
				break;
			// mask
			case 1:
				out_color = vec4(0.f, 1.f, 0.f, 1.f);
				break;
			// blend
			case 2:
				out_color = vec4(0.f, 0.f, 1.f, 1.f);
				break;
			}
			break;
			
		case 28: // alpha cutoff
			out_color = vec4(alpha_cutoff);
			break;
	
		case 29: // alpha mask
			out_color = vec4(0.f, 0.f, 0.f, 1.f);
			if(alpha_mode == 1)
			{
				// transparent
				if(base_color.a < alpha_cutoff)
				{
					out_color = vec4(1.f, 1.f, 1.f, 1.f);
				}
			}
			break;
			
		case 30: // double sidedness
			if (double_sided)
			{
				out_color = vec4(1.f, 1.f, 1.f, 1.f);
			}
			else
			{
				out_color = vec4(0.f, 0.f, 0.f, 1.f);
			}
			break;
	}
}