#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require
#include "global.glsl"

#include "global-descset.glsl"

#include "lights/pointlight.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) noperspective in vec2 uv;

// uniform

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput g_DepthInput;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput g_NormalInput;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput g_AlbedoInput;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput g_SpecularInput;
layout (input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput g_EmissiveInput;
layout (input_attachment_index = 5, set = 1, binding = 5) uniform subpassInput g_VelocityInput;
layout (input_attachment_index = 6, set = 1, binding = 6) uniform subpassInput g_UVDrawIndexInput;
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
