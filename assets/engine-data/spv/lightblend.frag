#version 460
#extension GL_GOOGLE_include_directive : enable
#include "attachments.glsl"
#include "global.glsl"
// out

layout(location = 0) out vec4 outColor;

// in

layout(location = 0) in vec2 uv;

// uniform

vec4 AmbientInfoBlurredOcclusion()
{
	float Offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );

    vec4 color = texture(AoSampler, uv);

    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            vec2 tc = uv;
            tc.x = uv.x + Offsets[j] / textureSize(AoSampler, 0).x;
            tc.y = uv.y + Offsets[i] / textureSize(AoSampler, 0).y;
            color.a += texture(AoSampler, tc).a;
        }
    }

    color.a /= 16.0;

    return color;
}

void main()
{
    vec3 emissive = texture(g_EmissiveSampler, uv).rgb;
	vec3 directLight = texture(directLightSampler, uv).rgb;
	vec3 indirectLight = texture(indirectLightSampler, uv).rgb;
	//vec3 indirectRtSpec = texture(indirectRaytracedSpecular, uv).rgb;
    vec3 mirror = texture(mirrorSampler, uv).rgb;
    vec4 ambientInfo = AmbientInfoBlurredOcclusion();
	// ...
	vec3 final =  directLight + (indirectLight * ambientInfo.a) + ambientInfo.rgb + emissive /* + mirror  + indirectRtSpec */;

	outColor = vec4(final, 1.0);
}
























