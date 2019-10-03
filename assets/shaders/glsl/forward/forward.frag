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
	
	vec4 light_fragpos;
} dataIn;

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
// this renderer currently supports a single directional light map
layout(binding=5) uniform sampler2D shadowMapSampler;

#define _1_PI 0.318309886183790671538f
#define PI 3.14159265358979323846f

float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapSampler, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	// cure shadow acne
	float bias = max(0.05 * (1.0 - dot(N, L)), 0.005); 
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	// TODO: peter panning, pcf, oversampling
    return shadow;
}  

vec3 FresnelSchlick(float cosTheta, vec3 F0)
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
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
	
	float NDF = DistributionGGX(N, H, roughness);       
	float G = GeometrySmith(N, V, L, roughness); 
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001); 
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;
	
	// light stuff
	//float distance = length(light_pos - dataIn.world_pos);
	//float attenuation = 1.0 / (distance * distance);
	float shadow = ShadowCalculation(dataIn.light_fragpos, N, L); 
	vec3 radiance = (1.0 - shadow) * light_color * light_intensity /** attenuation*/; 
	
	vec3 Lo = (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive /*+ ambient*/;
	color = mix(color, color * occlusion, occlusion_strength);
	
	out_color = vec4(color, 1.f);
}