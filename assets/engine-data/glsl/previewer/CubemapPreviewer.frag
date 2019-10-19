#version 460 core

out vec4 out_color;

in vec2 texCoord;

uniform vec3 wcs_viewPos;
uniform vec2 invTextureSize;

uniform mat4 vp_inv;

layout(binding=0) uniform sampler2D depthSampler;
layout(binding=1) uniform samplerCube cubemapSampler;

vec3 ReconstructWCS(vec2 uv)
{
	vec4 clipPos; // clip space reconstruction
	clipPos.x = 2.0 * texCoord.x - 1.0;
	clipPos.y = 2.0 * texCoord.y - 1.0;
	clipPos.z = 2.0 * texture(depthSampler, uv.xy).r -1.0;
	clipPos.w = 1.0;
	
	vec4 pwcs = vp_inv * clipPos; // clip space -> world space

	return pwcs.xyz / pwcs.w; // return world space pos xyz
}

void main()
{
	vec2 uv = gl_FragCoord.st * invTextureSize;

	vec3 dir = normalize(ReconstructWCS(uv) - wcs_viewPos);

	out_color = texture(cubemapSampler, dir);
}