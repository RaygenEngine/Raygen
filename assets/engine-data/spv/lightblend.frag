#include "global-descset.glsl"

#include "radiance.glsl"
#include "surface.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout(push_constant) uniform PC
{
	int quadlightCount;
};

layout(set = 1, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

float AmbienOcclusionBlurred()
{
	float Offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );

    float color = texture(ambientLightSampler, uv).a;

	// TODO: correct ambient occlusion and jitter blur

    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            vec2 tc = uv;
            tc.x = uv.x + Offsets[j] / textureSize(ambientLightSampler, 0).x;
            tc.y = uv.y + Offsets[i] / textureSize(ambientLightSampler, 0).y;
            color += texture(ambientLightSampler, tc).a;
        }
    }

    color /= 16.0;

    return color;
}

void main()
{
	float depth = texture(g_DepthSampler, uv).r;

	vec4 ambientInfo = texture(ambientLightSampler, uv);


	if(depth == 1.0) {
		outColor = vec4(ambientInfo.rgb, 1.0); // sky
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
		arealights += Arealight_EstimateDirectNoLightAttenuation(ql, surface) * arealightShadowing[i];
    }

	vec3 final =  directLight + (indirectLight * AmbienOcclusionBlurred()) + surface.emissive + mirror + arealights;

	outColor = vec4(final, 1.0);
}
