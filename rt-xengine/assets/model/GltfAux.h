#pragma once

#include "tinygltf/tiny_gltf.h"

namespace Assets
{
	inline GeometryMode GetGeometryModeFromGltf(int32 gltfMode)
	{
		switch (gltfMode)
		{
		case TINYGLTF_MODE_POINTS:         return GM_POINTS;
		case TINYGLTF_MODE_LINE:           return GM_LINE;
		case TINYGLTF_MODE_LINE_LOOP:      return GM_LINE_LOOP;
		case TINYGLTF_MODE_LINE_STRIP:     return GM_LINE_STRIP;
		case TINYGLTF_MODE_TRIANGLES:      return GM_TRIANGLES;
		case TINYGLTF_MODE_TRIANGLE_STRIP: return GM_TRIANGLE_STRIP;
		case TINYGLTF_MODE_TRIANGLE_FAN:   return GM_TRIANGLE_FAN;
		default:						   return GM_INVALID;
		}
	}

	inline TextureFiltering GetTextureFilteringFromGltf(int32 gltfFiltering)
	{
		switch (gltfFiltering)
		{
		case TINYGLTF_TEXTURE_FILTER_NEAREST:                return TF_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:                 return TF_LINEAR;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: return TF_NEAREST_MIPMAP_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:  return TF_LINEAR_MIPMAP_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:  return TF_NEAREST_MIPMAP_LINEAR;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:   return TF_LINEAR_MIPMAP_LINEAR;
			// not defined -> used linear					
		case -1:											 return TF_LINEAR;
		default:                                             return TF_INVALID;
		}
	};

	inline TextureWrapping GetTextureWrappingFromGltf(int32 gltfWrapping)
	{
		switch (gltfWrapping)
		{
		case TINYGLTF_TEXTURE_WRAP_REPEAT:          return TW_REPEAT;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:   return TW_CLAMP_TO_EDGE;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return TW_MIRRORED_REPEAT;
		default:                                    return TW_INVALID;
		}
	};

	inline AlphaMode GetAlphaModeFromGltf(const std::string& gltfAlphaMode)
	{
		if (Core::CaseInsensitiveCompare(gltfAlphaMode, "OPAQUE"))
			return AM_OPAQUE;
		if (Core::CaseInsensitiveCompare(gltfAlphaMode, "MASK"))
			return AM_MASK;
		if (Core::CaseInsensitiveCompare(gltfAlphaMode, "BLEND"))
			return AM_BLEND;

		return AM_INVALID;
	}

	inline BufferElementType GetElementTypeFromGltf(int32 gltfElementType)
	{
		switch (gltfElementType)
		{
		case TINYGLTF_TYPE_SCALAR: return BET_SCALAR;
		case TINYGLTF_TYPE_VEC2:   return BET_VEC2;
		case TINYGLTF_TYPE_VEC3:   return BET_VEC3;
		case TINYGLTF_TYPE_VEC4:   return BET_VEC4;
		case TINYGLTF_TYPE_MAT2:   return BET_MAT2;
		case TINYGLTF_TYPE_MAT3:   return BET_MAT3;
		case TINYGLTF_TYPE_MAT4:   return BET_MAT4;
		default:                   return BET_INVALID;
		}
	}

	inline BufferComponentType GetComponentTypeFromGltf(int32 gltfComponentType)
	{
		switch (gltfComponentType)
		{
		case TINYGLTF_COMPONENT_TYPE_BYTE:			 return BCT_BYTE;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:  return BCT_UNSIGNED_BYTE;
		case TINYGLTF_COMPONENT_TYPE_SHORT:			 return BCT_SHORT;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return BCT_UNSIGNED_SHORT;
		case TINYGLTF_COMPONENT_TYPE_INT:			 return BCT_INT;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:   return BCT_UNSIGNED_INT;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:			 return BCT_FLOAT;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE:		 return BCT_DOUBLE;
		default:									 return BCT_INVALID;
		}
	}
}
