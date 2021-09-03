#include "global-descset.glsl"

#include "radiance-rt.glsl"
#include "surface.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout(push_constant) uniform PC
{
	int quadlightCount;
};

layout(set = 1, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;
layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAs;

void main()
{
	float depth = texture(g_DepthSampler, uv).r;

	vec4 ambientInfo = texture(ambientLightSampler, uv);


	if(depth == 1.0) {
		outColor = vec4(ambientInfo.rgb, 1.0);
		return;
	}

    Surface surface = surfaceFromGBuffer(
	    cam,
	    depth,
	    g_SNormalSampler,
		g_GNormalSampler,
	    g_AlbedoSampler,
	    g_SpecularSampler,
	    g_EmissiveSampler,
	    uv
    );

	vec3 directLight = texture(directLightSampler, uv).rgb;
	vec3 indirectLight = texture(indirectLightSampler, uv).rgb;
    vec4 arealightShadowing = texture(_reserved0_, uv);
    vec3 mirror = texture(_reserved2_, uv).rgb;

    vec3 arealights = vec3(0);

    for (int i = 0; i < min(3, quadlightCount); ++i) {
		Quadlight ql = quadlights.light[i];
		arealights += Arealight_EstimateDirectNoLightAttenuation(topLevelAs, ql, surface) * arealightShadowing[i];
    }

	vec3 final =  directLight + (indirectLight *  ambientInfo.rgb) + surface.emissive + mirror + arealights;

	outColor = vec4(final, 1.0);
}
