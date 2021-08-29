#include "tonemapping.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout(set = 0, binding = 0) uniform sampler2D hdrColorSampler;

layout(push_constant) uniform PC
{
	float gamma;
	float exposure;
	int tonemapMode;
};

void main()
{
	vec3 hdrColor = texture(hdrColorSampler, uv).rgb;

	outColor = vec4(tonemap(hdrColor, tonemapMode, gamma, exposure), 1.0);
}
