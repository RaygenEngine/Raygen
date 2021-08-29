#include "global-descset.glsl"
#include "radiance.glsl"

#pragma begin

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput g_DepthInput;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput g_SNormalInput;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput g_GNormalInput;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput g_AlbedoInput;
layout (input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput g_SpecularInput;
layout (input_attachment_index = 5, set = 1, binding = 5) uniform subpassInput g_EmissiveInput;
layout (input_attachment_index = 6, set = 1, binding = 6) uniform subpassInput g_VelocityInput;
layout (input_attachment_index = 7, set = 1, binding = 7) uniform subpassInput g_UVDrawIndexInput;
layout(set = 2, binding = 0) uniform UBO_Spotlight { Spotlight light; };
layout(set = 3, binding = 0) uniform sampler2DShadow shadowmap;

void main() 
{
	float depth = subpassLoad(g_DepthInput).r;

	// PERF: volume
	if(depth == 1.0) {
		discard; 
	}

	Surface surface = surfaceFromGBuffer(
	    cam,
		depth,
		g_SNormalInput,
		g_GNormalInput,
		g_AlbedoInput,
		g_SpecularInput,
		g_EmissiveInput,
		uv
	);

	vec3 finalContribution = Spotlight_EstimateDirectSmooth(light, shadowmap, surface);
	outColor = vec4(finalContribution, 1);
}
