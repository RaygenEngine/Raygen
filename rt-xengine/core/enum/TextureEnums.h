#pragma once

typedef enum
{
	TF_NEAREST,
	TF_LINEAR,
	TF_NEAREST_MIPMAP_NEAREST,
	TF_NEAREST_MIPMAP_LINEAR,
	TF_LINEAR_MIPMAP_NEAREST,
	TF_LINEAR_MIPMAP_LINEAR,
	TF_INVALID
} TextureFiltering;

typedef enum
{
	TW_CLAMP_TO_EDGE,
	TW_MIRRORED_REPEAT,
	TW_REPEAT,
	TW_INVALID
} TextureWrapping;

typedef enum
{
	CMF_RIGHT = 0,
	CMF_LEFT,
	CMF_UP,
	CMF_DOWN,
	CMF_FRONT,
	CMF_BACK,
	CMF_COUNT
} CubeMapFace;

// Low dynamic range textures are loaded as byte (8 bits per channel)
// High dynamic range textures are loaded as float32 (32 bits per channel)
// TODO: expand this
typedef enum
{
	DR_LOW = 0,
	DR_HIGH
} DynamicRange;

inline const char* TexelEnumToString(DynamicRange dr)
{
	switch (dr)
	{
	case DR_LOW: return "LDR";
	case DR_HIGH: return "HDR";
	default:  return "invalid";
	}
}
