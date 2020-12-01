#version 460 
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"
#include "lights/reflprobe.glsl"
#include "mainpass-inputs.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) noperspective in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform UBO_Reflprobe { Reflprobe rp; };
layout(set = 3, binding = 0) uniform samplerCube environmentSampler;
layout(set = 4, binding = 0) uniform samplerCube irradianceSampler;
layout(set = 5, binding = 0) uniform samplerCube prefilteredSampler;
layout(set = 6, binding = 10) uniform sampler2D std_BrdfLut; // TODO: globally accessible from ubo

void main( ) 
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

	// for preview
//	if(surface.depth == 1.0) {
//	
//		// Don't use surfaceOutgoingLightDirDir, change of basis breaks at inf depth
//		vec3 V = normalize(surface.position - cam.position);
//		outColor = textureLod(prefilteredSampler, V, 0);
//		return; 
//	}
	
	vec3 finalContribution = Reflprobe_Contribution(rp, std_BrdfLut, irradianceSampler, prefilteredSampler, surface);
	outColor = vec4(finalContribution, 1);
}
