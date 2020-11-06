#version 460
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"
#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

vec3 simpleBlurAO()
{
	float Offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );

    vec3 color = vec3(0.0, 0.0, 0.0);

    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            vec2 tc = uv;
            tc.x = uv.x + Offsets[j] / textureSize(ray_AOSampler, 0).x;
            tc.y = uv.y + Offsets[i] / textureSize(ray_AOSampler, 0).y;
            color += texture(ray_AOSampler, tc).xyz;
        }
    }

    color /= 16.0;

    return color;
}

void main() {
	vec3 raster_DirectLight = texture(raster_DirectLightSampler, uv).rgb;
	vec3 raster_IBLminusMirrorReflections = texture(raster_IBLminusMirrorReflectionsSampler, uv).rgb;

	vec3 ray_MirrorReflections = texture(ray_MirrorReflectionsSampler, uv).rgb;
	//vec3 ray_AO = simpleBlurAO();

	vec3 finalRes = raster_DirectLight + (raster_IBLminusMirrorReflections);// + ray_MirrorReflections) * ray_AO;

	outColor = vec4(finalRes, 1.0);
}                               





