#pragma once

#include "core/Types.h"

enum class LogLevelTarget
{
	TRACE = 0,
	DEBUG,
	INFO,
	WARN,
	ERR,
	CRITICAL,
	OFF
};

enum class ComponentType
{
	CHAR,
	BYTE,
	SHORT,
	USHORT,
	INT,
	UINT,
	FLOAT,
	DOUBLE
};

enum class ElementType
{
	SCALAR,
	VEC2,
	VEC3,
	VEC4,
	MAT2,
	MAT3,
	MAT4
};

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

enum class TextureTarget
{
	TEXTURE_1D,
	TEXTURE_2D,
	TEXTURE_3D,
	TEXTURE_ARRAY,
	TEXTURE_CUBEMAP,
	TEXTURE_CUBEMAP_ARRAY
};

struct CubemapFace {
	enum : int32
	{
		RIGHT = 0,
		LEFT,
		UP,
		DOWN,
		FRONT,
		BACK,
		COUNT
	};
};

namespace utl {

inline int32 ComponentTypeByteCount(ComponentType bct)
{
	switch (bct) {
		case ComponentType::CHAR:
		case ComponentType::BYTE: return 1;
		case ComponentType::SHORT:
		case ComponentType::USHORT: return 2;
		case ComponentType::INT:
		case ComponentType::UINT:
		case ComponentType::FLOAT: return 4;
		case ComponentType::DOUBLE: return 8;
		default: return -1;
	}
}

inline int32 ElementComponentCount(ElementType bet)
{
	switch (bet) {
		case ElementType::SCALAR: return 1;
		case ElementType::VEC2: return 2;
		case ElementType::VEC3: return 3;
		case ElementType::VEC4:
		case ElementType::MAT2: return 4;
		case ElementType::MAT3: return 9;
		case ElementType::MAT4: return 16;
		default: return -1;
	}
}
} // namespace utl
