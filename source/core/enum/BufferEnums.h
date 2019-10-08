#pragma once

enum class BufferComponentType {
	BYTE,
	UNSIGNED_BYTE,
	SHORT,
	UNSIGNED_SHORT,
	INT,
	UNSIGNED_INT,
	FLOAT,
	DOUBLE,
	INVALID
};

enum class BufferElementType { SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4, INVALID };

namespace utl {
inline int32 GetComponentTypeByteCount(BufferComponentType bct)
{
	switch (bct) {
		case BufferComponentType::BYTE: return 1;
		case BufferComponentType::UNSIGNED_BYTE: return 1;
		case BufferComponentType::SHORT: return 2;
		case BufferComponentType::UNSIGNED_SHORT: return 2;
		case BufferComponentType::INT: return 4;
		case BufferComponentType::UNSIGNED_INT: return 4;
		case BufferComponentType::FLOAT: return 4;
		case BufferComponentType::DOUBLE: return 8;
		default: return -1;
	}
}

inline int32 GetElementComponentCount(BufferElementType bet)
{
	switch (bet) {
		case BufferElementType::SCALAR: return 1;
		case BufferElementType::VEC2: return 2;
		case BufferElementType::VEC3: return 3;
		case BufferElementType::VEC4: return 4;
		case BufferElementType::MAT2: return 4;
		case BufferElementType::MAT3: return 9;
		case BufferElementType::MAT4: return 16;
		default: return -1;
	}
}
} // namespace utl