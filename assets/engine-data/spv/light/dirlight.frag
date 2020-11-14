#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

#include "attachments.glsl"
#include "bsdf.glsl"
#include "lights/dirlight.glsl"
#include "onb.glsl"
#include "sampling.glsl"
#include "shadow.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform UBO_Dirlight { Dirlight light; };
layout(set = 3, binding = 0) uniform sampler2DShadow shadowmap;

void main() 
{
	Surface surface = surfaceFromGBuffer(
	    cam,
		g_DepthSampler,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv
	);

	vec3 finalContribution = Dirlight_SmoothContribution(light, shadowmap, surface);
	outColor = vec4(finalContribution, 1);
}                               
                                
                                 


