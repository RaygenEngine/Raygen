#version 460
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

void main() {
	vec3 raster_DirectLight = texture(raster_DirectLightSampler, uv).rgb;
	vec3 raster_IBLminusMirrorReflections = texture(raster_IBLminusMirrorReflectionsSampler, uv).rgb;

	vec3 ray_MirrorReflections = texture(ray_MirrorReflectionsSampler, uv).rgb;
	vec3 ray_AO = texture(ray_AOSampler, uv).rgb;

	vec3 finalRes = raster_DirectLight + (raster_IBLminusMirrorReflections + ray_MirrorReflections) * ray_AO;
	//finalRes = raster_DirectLight + (raster_IBLminusMirrorReflections + ray_MirrorReflections);
	//finalRes = (raster_DirectLight + raster_IBLminusMirrorReflections + ray_MirrorReflections);
	outColor = vec4(finalRes, 1.0);
}                               




