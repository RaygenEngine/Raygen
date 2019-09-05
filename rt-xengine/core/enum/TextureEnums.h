#pragma once

enum class TextureFiltering
{
	NEAREST,
	LINEAR,
	NEAREST_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_NEAREST,
	LINEAR_MIPMAP_LINEAR
};

enum class TextureWrapping
{
	CLAMP_TO_EDGE,
	MIRRORED_REPEAT,
	REPEAT
};

enum CubeMapFace : int32
{
	CMF_RIGHT = 0,
	CMF_LEFT,
	CMF_UP,
	CMF_DOWN,
	CMF_FRONT,
	CMF_BACK,
	CMF_COUNT
};

// Low dynamic range textures are loaded as byte (8 bits per channel)
// High dynamic range textures are loaded as float32 (32 bits per channel)
// TODO: expand this
enum class DynamicRange
{
	LOW = 0,
	HIGH
};

inline const char* TexelEnumToString(DynamicRange dr)
{
	switch (dr)
	{
	case DynamicRange::LOW: return "LDR";
	case DynamicRange::HIGH: return "HDR";
	default:  return "invalid";
	}
}
