#version 460 
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"
// WIP:
layout(set = 3, binding = 10) uniform sampler2D std_BrdfLut;
#include "lights/reflprobe.glsl"
#include "mainpass-inputs.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) noperspective in vec2 uv;

// uniform

layout(push_constant) uniform PC {
	mat4 reflVolMatVP;
    vec4 reflPosition;
    int lodCount;
} push;

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform samplerCube skyboxSampler;
layout(set = 2, binding = 1) uniform samplerCube irradianceSampler;
layout(set = 2, binding = 2) uniform samplerCube prefilteredSampler;

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
	///if(surface.depth == 1.0) {
	
		// Don't use surfaceOutgoingLightDirDir, change of basis breaks at inf depth
		//V = normalize(cam.position - surface.position);
	
		//outColor = sampleCubemapLH(skyboxSampler, normalize(-V));
		//return; 
	//}
	
	vec3 finalContribution = Reflprobe_Contribution(irradianceSampler, prefilteredSampler, push.lodCount, surface);
	outColor = vec4(finalContribution, 1);
}













