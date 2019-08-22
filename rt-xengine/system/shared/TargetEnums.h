#ifndef TARGETENUMS_H
#define TARGETENUMS_H

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
	ST_GL_TEXTURE = 0,
	ST_GL_OCULUS_SINGLE_TEXTURE,
	ST_GL_SURFACE,
	ST_VK_SURFACE,
	ST_MAPPED_MEMORY,
	ST_NONE
} SurfaceTarget;

typedef enum
{
	TC_RED = 0,
	TC_GREEN,
	TC_BLUE,
	TC_ALPHA
} TargetChannel;

typedef enum
{
	PM_ALBEDO = 0,
	PM_EMISSION,
	PM_REFLECTIVITY,
	PM_ROUGHNESS,
	PM_METAL,
	PM_WORLD_NORMAL,
	PM_NORMAL,
	PM_FINAL_NORMAL,
	PM_UV,
	PM_HEIGHT,
	PM_TRANSLUCENCY,
	PM_AMBIENT_OCCLUSION,
	PM_OPACITY,
	PM_NOISE,
	PM_COUNT
} SurfacePreviewTargetMode;

inline const char* SurfacePreviewTargetModeString(int32 e)
{
	switch (e)
	{
		case PM_ALBEDO: return "Albedo";
		case PM_EMISSION: return "Emission";
		case PM_REFLECTIVITY: return "Reflectivity";
		case PM_ROUGHNESS: return "Roughness";
		case PM_METAL: return "Metallic";
		case PM_WORLD_NORMAL: return "Normal";
		case PM_NORMAL: return "Normal map";
		case PM_FINAL_NORMAL: return "Final Normal";
		case PM_UV: return "UV";
		case PM_HEIGHT: return "Height";
		case PM_TRANSLUCENCY: return "Translucency";
		case PM_AMBIENT_OCCLUSION: return "Ambient Occlusion";
		case PM_OPACITY: return "Opacity";
		case PM_NOISE: return "Noise";
		default: return "Invalid";
	}
}

#endif // TARGETENUMS_H
