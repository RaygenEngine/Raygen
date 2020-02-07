#version 460 core

out vec4 out_color;

in vec2 uv;

uniform vec3 wcs_viewPos;

uniform mat4 vp_inv;

uniform vec3 ambient;

layout(binding=0) uniform sampler2D depthSampler;
layout(binding=1) uniform samplerCube skyboxSampler;
layout(binding=2) uniform sampler2D albedoSampler;
layout(binding=3) uniform sampler2D emissiveSampler;
layout(binding=4) uniform sampler2D specularSampler;

vec3 ReconstructWCS(vec2 uv)
{
	vec4 clipPos; // clip space reconstruction
	clipPos.x = 2.0 * uv.x - 1.0;
	clipPos.y = 2.0 * uv.y - 1.0;
	clipPos.z = texture(depthSampler, uv.xy).r;
	clipPos.w = 1.0;
	
	vec4 pwcs = vp_inv * clipPos; // clip space -> world space

	return pwcs.xyz / pwcs.w; // return world space pos xyz
}

void main()
{
	float currentDepth = texture(depthSampler, uv.xy).r;

	out_color = vec4(vec3(0.0), 1.0);

	if(currentDepth == 1.0)
	{
		vec3 dir = normalize(ReconstructWCS(uv) - wcs_viewPos);
		
		out_color = texture(skyboxSampler, dir);
		return;
	}
	
	vec3 albedo = texture(albedoSampler, uv.xy).rgb;
	vec3 emissive = texture(emissiveSampler, uv.xy).rgb;
	vec4 specular = texture(specularSampler, uv.xy);
	
	vec3 color = (albedo *  ambient) + emissive;
	color = mix(color, color * specular.b, specular.a);
	
	out_color += vec4(color, 1);
}
