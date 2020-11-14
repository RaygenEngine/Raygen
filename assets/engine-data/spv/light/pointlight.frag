#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require

#include "global.glsl"

#include "attachments.glsl"
#include "lights/pointlight.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform UBO_Pointlight { Pointlight pl; };
layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;

void main()
{
	vec2 iuv = gl_FragCoord.xy;
	ivec2 screenSize = textureSize(g_AlbedoSampler, 0);

	vec2 uv = iuv / screenSize; 

	Surface surface = surfaceFromGBuffer(
	    cam,
		g_DepthSampler,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv
	);

	vec3 finalContribution = Pointlight_SmoothContribution(topLevelAs, pl, surface);
	outColor = vec4(finalContribution, 1);
}                               



















