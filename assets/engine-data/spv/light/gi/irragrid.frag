#version 460 
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_nonuniform_qualifier : enable
#include "global.glsl"

#include "mainpass-inputs.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout (location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform UBO_Irragrid { Irragrid grid; };
layout(set = 3, binding = 0) uniform samplerCube irradianceSamplers[];

#include "lights/irragrid.glsl"

void main( ) {

	Surface surface = surfaceFromGBuffer(
	    cam,
		g_DepthInput,
		g_NormalInput,
		g_AlbedoInput,
		g_SpecularInput,
		g_EmissiveInput,
		uv
	);
	
	// PERF: remove when volume based rendering
	if(surface.depth == 1.0) {
		discard;
	}

	vec3 finalContribution = Irragrid_Contribution(grid, surface);
	outColor = vec4(finalContribution, 1);
}
