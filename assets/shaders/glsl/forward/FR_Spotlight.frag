#version 460 core

out vec4 out_color;
  
in Data
{ 
	vec3 tangent_frag_pos;
	vec3 tangent_view_pos;
	
	vec3 tangent_light_pos;	
	vec3 tangent_light_dir;
	
	vec2 text_coord[2];
	
	vec4 light_frag_pos;
} dataIn;


uniform struct SpotLight
{
	vec3 world_pos;
	vec3 world_dir;
	
	float outer_cut_off;
	float inner_cut_off;
	
	vec3 color;
	float intensity;
	
	float near;
	
	int atten_coef;

	mat4 vp;
} spot_light;

uniform vec3 ambient;

uniform vec4 base_color_factor;
uniform vec3 emissive_factor;
uniform float metallic_factor;
uniform float roughness_factor;
uniform float normal_scale;
uniform float occlusion_strength;
uniform float alpha_cutoff;
uniform bool mask;

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
layout(binding=5) uniform sampler2D spotLightMapSampler;

#define PI 3.14159265358979323846f

float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 L)
{	
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
	
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	
	if(fragPosLightSpace.z < spot_light.near + 0.005)
		return 1.0;
	
	// cure shadow acne
	float bias = 0.005;//max(0.05 * (1.0 - max(dot(N, L), 0.0)), 0.005); 
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(spotLightMapSampler, 0);
	int n = 1;
	for(int x = -n; x <= n; ++x)
	{
		for(int y = -n; y <= n; ++y)
		{
		    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
			float pcfDepth = texture(spotLightMapSampler, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	int count = 2*n + 1;
	shadow /= count * count;
		
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
	vec4 sampled_base_color = texture(baseColorSampler, dataIn.text_coord[base_color_texcoord_index]);
	
	float opacity = sampled_base_color.a * base_color_factor.a;
	// mask mode and cutoff
	if(mask && opacity < alpha_cutoff)
		discard;
	
	vec4 sampled_metallic_roughness = texture(metallicRoughnessSampler, dataIn.text_coord[metallic_roughness_texcoord_index]);
	vec4 sampled_emissive = texture(emissiveSampler, dataIn.text_coord[emissive_texcoord_index]);
	vec4 sampled_sample_normal = texture(normalSampler, dataIn.text_coord[normal_texcoord_index]);
	vec4 sampled_occlusion = texture(occlusionSampler, dataIn.text_coord[occlusion_texcoord_index]);
	
	// final material values
	vec3 albedo = sampled_base_color.rgb * base_color_factor.rgb;
	float metallic = sampled_metallic_roughness.b * metallic_factor;
	float roughness = sampled_metallic_roughness.g * roughness_factor;
	vec3 emissive = sampled_emissive.rgb * emissive_factor;
	float occlusion = sampled_occlusion.r;
	vec3 normal = sampled_sample_normal.rgb;

	// note: don't forget this is a normal map, normals are in tangent space
	// all calculations are in tangent space
	vec3 N = normalize((normal * 2.0 - 1.0) * vec3(normal_scale, normal_scale, 1.0));
	
	vec3 V = normalize(dataIn.tangent_view_pos - dataIn.tangent_frag_pos);
	
	// loop lights
	vec3 L = normalize(dataIn.tangent_light_pos - dataIn.tangent_frag_pos); 
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
	
	// attenuation
	float distance = length(dataIn.tangent_light_pos - dataIn.tangent_frag_pos);
	float attenuation = 1.0 / pow(distance, spot_light.atten_coef);
	
    // spotlight (soft edges)
	float theta = dot(L, normalize(-dataIn.tangent_light_dir));
    float epsilon = (spot_light.inner_cut_off - spot_light.outer_cut_off);
    float intensity = clamp((theta - spot_light.outer_cut_off) / epsilon, 0.0, 1.0);
	 
	float shadow = ShadowCalculation(dataIn.light_frag_pos, N, L); 
	vec3 radiance = ((1.0 - shadow) * spot_light.color * spot_light.intensity * attenuation * intensity); 

	vec3 Lo = (kD * albedo / PI + specular) * (radiance + ambient) * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, occlusion_strength);
	
	out_color = vec4(color, 1.f);
}