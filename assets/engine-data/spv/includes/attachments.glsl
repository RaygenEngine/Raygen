#ifndef attachments_glsl
#define attachments_glsl

// GBuffer
layout(set = 0, binding = 0) uniform sampler2D g_DepthSampler;
layout(set = 0, binding = 1) uniform sampler2D g_NormalSampler;
layout(set = 0, binding = 2) uniform sampler2D g_AlbedoSampler;
layout(set = 0, binding = 3) uniform sampler2D g_SpecularSampler;
layout(set = 0, binding = 4) uniform sampler2D g_EmissiveSampler;
layout(set = 0, binding = 5) uniform sampler2D g_VelocitySampler;
layout(set = 0, binding = 6) uniform sampler2D g_UVDrawIndexSampler;

layout(set = 0, binding = 7) uniform sampler2D std_BrdfLut;
                           
layout(set = 0, binding = 8) uniform sampler2D raster_DirectLightSampler;
											                          
layout(set = 0, binding = 9) uniform sampler2D raster_IBLminusMirrorReflectionsSampler;

layout(set = 0, binding = 10) uniform sampler2D ray_MirrorReflectionsSampler;

layout(set = 0, binding = 11) uniform sampler2D ray_AOSampler;

// Blend Rast + Ray                          
layout(set = 0, binding = 12) uniform sampler2D sceneColorSampler;

#endif