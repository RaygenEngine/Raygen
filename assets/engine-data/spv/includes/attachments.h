#ifndef attachments_h
#define attachments_h

// GBuffer
layout(set = 0, binding = 0) uniform sampler2D g_DepthSampler;
layout(set = 0, binding = 1) uniform sampler2D g_NormalSampler;
layout(set = 0, binding = 2) uniform sampler2D g_ColorSampler;
layout(set = 0, binding = 3) uniform sampler2D g_MRROSampler; // Metallic Roughness Reflectance Occlusion
layout(set = 0, binding = 4) uniform sampler2D g_EmissiveSampler;
											 
// Raster Direct                             
layout(set = 0, binding = 5) uniform sampler2D rasterDirectSampler;
											 
// RayTracing                                
layout(set = 0, binding = 6) uniform sampler2D rtIndirectSampler;
											 
// Blend Rast + Ray                          
layout(set = 0, binding = 7) uniform sampler2D sceneColorSampler;

#endif