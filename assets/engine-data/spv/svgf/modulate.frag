#include "global-descset.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput svgfAtrousFiltered;

void main() {
	
	// TODO: add albedo / emissive demodulation during pathtracing first bounce direct, and modulate here
	outColor = subpassLoad(svgfAtrousFiltered);
	return;

//	float depth = texture(g_DepthSampler, uv).r;
//
//	vec4 color = subpassLoad(svgfAtrousFiltered);
//
//	if(depth == 1.0) {
//		outColor = color;
//		return;
//	}
//
//	vec3 albedo = texture(g_AlbedoSampler, uv).xyz;
//    vec3 emissive = texture(g_EmissiveSampler, uv).xyz;
//
//	outColor = vec4(color.xyz * albedo + emissive, 1.f);
}
