#version 460
#extension GL_GOOGLE_include_directive : enable
#include "attachments.glsl"
#include "global.glsl"
// out

layout(location = 0) out vec4 outColor;

// in

layout(location = 0) in vec2 uv;

// uniform

void main()
{
	vec3 directLight = texture(directLightSampler, uv).rgb;
	vec3 indirectLight = texture(indirectLightSampler, uv).rgb;
	vec3 indirectRtSpec = texture(indirectRaytracedSpecular, uv).rgb;
	// ...
	vec3 final = directLight + indirectLight;

	outColor = vec4(final, 1.0);
}








