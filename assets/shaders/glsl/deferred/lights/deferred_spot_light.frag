#version 460 core

out vec4 out_color;

in vec2 uv;

uniform vec3 view_pos;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform float light_intensity;

uniform mat4 light_space_matrix;

layout(binding=0) uniform sampler2D positionsSampler;
layout(binding=1) uniform sampler2D normalsSampler;
layout(binding=2) uniform sampler2D albedoOpacitySampler;
layout(binding=3) uniform sampler2D metallicRoughnessOcclusionOcclusionStrengthSampler;
layout(binding=4) uniform sampler2D emissiveSampler;

layout(binding=5) uniform sampler2D shadowMapSampler;

#define _1_PI 0.318309886183790671538f
#define PI 3.14159265358979323846f

float ShadowCalculation(vec3 pos, vec3 N, vec3 L)
{
	// PERF:
	vec4 fragPosLightSpace = light_space_matrix * vec4(pos, 1.0);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapSampler, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	// cure shadow acne
	float bias = 0.005;//max(0.05 * (1.0 - dot(N, L)), 0.005); 
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
	// material values
	vec3 pos = texture(positionsSampler, uv).rgb;
	vec3 N = texture(normalsSampler, uv).rgb;
	vec3 albedo = texture(albedoOpacitySampler, uv).rgb;
	float opacity = texture(albedoOpacitySampler, uv).a;
	float metallic = texture(metallicRoughnessOcclusionOcclusionStrengthSampler, uv).r;
	float roughness = texture(metallicRoughnessOcclusionOcclusionStrengthSampler, uv).g;
	float occlusion = texture(metallicRoughnessOcclusionOcclusionStrengthSampler, uv).b;
	float occlusion_strength = texture(metallicRoughnessOcclusionOcclusionStrengthSampler, uv).a;
	vec3 emissive = texture(emissiveSampler, uv).rgb;
	
	vec3 V = normalize(view_pos - pos);
	vec3 L = normalize(light_pos - pos); 
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
	
	float distance = length(light_pos - pos);
	float attenuation = 1.0 / (distance * distance);
	float shadow = ShadowCalculation(pos, N, L); 
	vec3 radiance = (1.0 - shadow) * light_color * light_intensity * attenuation; 
	
	vec3 Lo = (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);

	vec3 color = Lo + emissive;
	color = mix(color, color * occlusion, occlusion_strength);
	
	out_color = vec4(color, 1.f);
}