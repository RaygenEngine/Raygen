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

#define _1_PI 0.318309886183790671538f
#define PI 3.14159265358979323846f

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// very basic unoptimized surface test shader
void main()
{
	// sample material textures
	vec4 sampled_base_color = texture(baseColorSampler, dataIn.text_coord0);
	vec4 sampled_metallic_roughness = texture(metallicRoughnessSampler, dataIn.text_coord0);
	vec4 sampled_emissive = texture(emissiveSampler, dataIn.text_coord0);
	vec4 sampled_sample_normal = texture(normalSampler, dataIn.text_coord0);
	vec4 sampled_occlusion = texture(occlusionSampler, dataIn.text_coord0);
	
	// final material values
	vec3 albedo = sampled_base_color.rgb * base_color_factor.rgb;
	float opacity = sampled_base_color.a * base_color_factor.a;
	float metallic = sampled_metallic_roughness.b * metallic_factor;
	float roughness = sampled_metallic_roughness.g * roughness_factor;
	vec3 emissive = sampled_emissive.rgb * emissive_factor;
	float occlusion = sampled_occlusion.r;
	vec3 normal = sampled_sample_normal.rgb;

	// mask mode and cutoff, TODO: handle other modes as well
	if(alpha_mode == 1 && opacity < alpha_cutoff)
		discard;
		
	// vectors

	// TODO: all calculations in tangent space
	vec3 N = normalize((normal.rgb * 2.0 - 1.0) * vec3(normal_scale, normal_scale, 1.0));
	N = normalize(dataIn.TBN * N);
	vec3 V = normalize(view_pos - dataIn.world_pos);
	
	// loop lights
	vec3 L = normalize(light_pos - dataIn.world_pos); 
	vec3 H = normalize(V + L);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
	
	float NDF = DistributionGGX(N, H, roughness);       
	float G = GeometrySmith(N, V, L, roughness); 
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001); 
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;
	
	// light stuff
	float distance = length(light_pos - dataIn.world_pos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = light_color * light_intensity * attenuation; 
	
	vec3 Lo = (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive /*+ ambient*/;
	color = mix(color, color * occlusion, occlusion_strength);
	
	//color = color / (color + vec3(1.0));
    //color = pow(color, vec3(1.0/2.2));
	
	out_color = vec4(color, 1.f);

	// TODO: different rendering code path
	/*switch (mode-1)
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
	}*/
}