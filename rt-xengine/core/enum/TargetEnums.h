#pragma once

typedef enum
{
	LLT_TRACE = 0,
	LLT_DEBUG,
	LLT_INFO,
	LLT_WARN,
	LLT_ERROR,
	LLT_CRITICAL,
	LLT_OFF
} LogLevelTarget;

typedef enum
{
	CT_RED = 0,
	CT_GREEN,
	CT_BLUE,
	CT_ALPHA
} ChannelTarget;

typedef enum
{
	PT_ALBEDO = 0,
	PT_METALLIC,
	PT_ROUGHNESS,
	PT_NORMAL,
	PT_TANGENT,
	PT_BITANGENT,
	PT_UV,
	PT_HEIGHT,
	PT_TRANSLUCENCY,
	PT_OCCLUSION,
	PT_OPACITY,
	PT_COUNT
} PreviewTarget;

// TODO
//inline const char* SurfacePreviewTargetModeString(int32 e)
//{
//	switch (e)
//	{
//		case PM_ALBEDO: return "Albedo";
//		case PM_EMISSION: return "Emission";
//		case PM_REFLECTIVITY: return "Reflectivity";
//		case PM_ROUGHNESS: return "Roughness";
//		case PM_METAL: return "Metallic";
//		case PM_WORLD_NORMAL: return "Normal";
//		case PM_NORMAL: return "Normal map";
//		case PM_FINAL_NORMAL: return "Final Normal";
//		case PM_UV: return "UV";
//		case PM_HEIGHT: return "Height";
//		case PM_TRANSLUCENCY: return "Translucency";
//		case PM_AMBIENT_OCCLUSION: return "Ambient Occlusion";
//		case PM_OPACITY: return "Opacity";
//		case PM_NOISE: return "Noise";
//		default: return "Invalid";
//	}
//}
//// TODO ADD ANY 
