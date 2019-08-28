#pragma once

typedef enum
{
	BCT_BYTE,
	BCT_UNSIGNED_BYTE,
	BCT_SHORT,
	BCT_UNSIGNED_SHORT,
	BCT_INT,
	BCT_UNSIGNED_INT,
	BCT_FLOAT,
	BCT_DOUBLE,
	BCT_INVALID
} BufferComponentType;

inline int32 GetComponentTypeByteCount(BufferComponentType bct)
{
	switch (bct)
	{
	case BCT_BYTE:           return 1;
	case BCT_UNSIGNED_BYTE:  return 1;
	case BCT_SHORT:          return 2;
	case BCT_UNSIGNED_SHORT: return 2;
	case BCT_INT:            return 2;
	case BCT_UNSIGNED_INT:   return 4;
	case BCT_FLOAT:          return 4;
	case BCT_DOUBLE:         return 8;
	default:                 return -1;
	}
}

typedef enum
{
	BET_SCALAR,
	BET_VEC2,
	BET_VEC3,
	BET_VEC4,
	BET_MAT2,
	BET_MAT3,
	BET_MAT4,
	BET_INVALID
} BufferElementType;

inline int32 GetElementComponentCount(BufferElementType bet)
{
	switch (bet)
	{
	case BET_SCALAR: return 1;
	case BET_VEC2:   return 2;
	case BET_VEC3:   return 3;
	case BET_VEC4:   return 4;
	case BET_MAT2:   return 4;
	case BET_MAT3:   return 9;
	case BET_MAT4:   return 16;
	default:         return -1;
	}
}
