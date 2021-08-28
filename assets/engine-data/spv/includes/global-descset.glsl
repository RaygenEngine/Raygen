#ifndef global_descset_glsl
#define global_descset_glsl

// GBuffer + direct and indirect light
layout(set = 0, binding = 0) uniform sampler2D g_DepthSampler;
layout(set = 0, binding = 1) uniform sampler2D g_SNormalSampler;
layout(set = 0, binding = 2) uniform sampler2D g_GNormalSampler;
layout(set = 0, binding = 3) uniform sampler2D g_AlbedoSampler;
layout(set = 0, binding = 4) uniform sampler2D g_SpecularSampler;
layout(set = 0, binding = 5) uniform sampler2D g_EmissiveSampler;
layout(set = 0, binding = 6) uniform sampler2D g_VelocitySampler;
layout(set = 0, binding = 7) uniform sampler2D g_UVDrawIndexSampler;
layout(set = 0, binding = 8) uniform sampler2D directLightSampler;
layout(set = 0, binding = 9) uniform sampler2D indirectLightSampler;

// Ambient
layout(set = 0, binding = 10) uniform sampler2D ambientLightSampler;

// BRDF Lut
layout(set = 0, binding = 11) uniform sampler2D std_BrdfLut;

// Reserved
layout(set = 0, binding = 12) uniform sampler2D _reserved0_;
layout(set = 0, binding = 13) uniform sampler2D _reserved1_;
layout(set = 0, binding = 14) uniform sampler2D _reserved2_;

// Scene color
layout(set = 0, binding = 15) uniform sampler2D sceneColorSampler;

// Viewer
layout(set = 0, binding = 16) uniform UBO_Camera { Camera cam; };

#endif
