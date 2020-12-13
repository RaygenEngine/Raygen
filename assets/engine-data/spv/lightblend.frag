#version 460
#extension GL_GOOGLE_include_directive : enable
#include "global.glsl"

#include "global-descset.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in

layout(location = 0) in vec2 uv;

// uniform

layout(push_constant) uniform PC
{
	int quadlightCount;
};

layout(set = 1, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

vec4 AmbientInfoBlurredOcclusion()
{
	float Offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );

    vec4 color = texture(aoSampler, uv);

    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            vec2 tc = uv;
            tc.x = uv.x + Offsets[j] / textureSize(aoSampler, 0).x;
            tc.y = uv.y + Offsets[i] / textureSize(aoSampler, 0).y;
            color.a += texture(aoSampler, tc).a;
        }
    }

    color.a /= 16.0;

    return color;
}

vec3 Quadlight_AfterContribution(Quadlight ql, Surface surface)
{
	vec3 L = normalize(ql.center - surface.position);
	addIncomingLightDirection(surface, L);
		

	return ql.color * ql.intensity * DirectLightBRDF(surface); // we use the surface.nol calculated in arealights
}

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

	vec3 directLight = texture(directLightSampler, uv).rgb;
	vec3 indirectLight = texture(indirectLightSampler, uv).rgb;
    vec4 arealightShadowing = texture(_reserved0_, uv);
    vec3 mirror = texture(mirrorSampler, uv).rgb;
    vec4 ambientInfo = AmbientInfoBlurredOcclusion();
	// ...

    vec3 arealights = vec3(0);

    for (int i = 0; i < quadlightCount; ++i) {
		Quadlight ql = quadlights.light[i];
		arealights += Quadlight_AfterContribution(ql, surface) * arealightShadowing[i];
    }

	vec3 final =  directLight + (indirectLight * ambientInfo.a) + ambientInfo.rgb + surface.emissive + mirror + arealights;
    vec3 pt = texture(_reserved1_, uv).rgb;
	outColor = vec4(pt, 1.0);
}
