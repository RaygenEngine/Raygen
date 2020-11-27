#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require

#include "global.glsl"

#include "lights/pointlight.glsl"
#include "mainpass-inputs.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) noperspective in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform UBO_Pointlight { Pointlight pl; };
layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;

void main()
{
	Surface surface = surfaceFromGBuffer(
	    cam,
		g_DepthInput,
		g_NormalInput,
		g_AlbedoInput,
		g_SpecularInput,
		g_EmissiveInput,
		uv
	);

	vec3 finalContribution = Pointlight_SmoothContribution(topLevelAs, pl, surface);
	outColor = vec4(finalContribution, 1);
}
