#pragma once

#include <tinygltf/tiny_gltf.h>

namespace gltfaux {
inline GeometryMode GetGeometryMode(int32 gltfMode)
{
	switch (gltfMode) {
		case TINYGLTF_MODE_POINTS: return GeometryMode::POINTS;
		case TINYGLTF_MODE_LINE: return GeometryMode::LINE;
		case TINYGLTF_MODE_LINE_LOOP: return GeometryMode::LINE_LOOP;
		case TINYGLTF_MODE_LINE_STRIP: return GeometryMode::LINE_STRIP;
		case TINYGLTF_MODE_TRIANGLES: return GeometryMode::TRIANGLES;
		case TINYGLTF_MODE_TRIANGLE_STRIP: return GeometryMode::TRIANGLE_STRIP;
		case TINYGLTF_MODE_TRIANGLE_FAN: return GeometryMode::TRIANGLE_FAN;
		default: return GeometryMode::TRIANGLES;
	}
}

inline TextureFiltering GetTextureFiltering(int32 gltfFiltering)
{
	switch (gltfFiltering) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST: return TextureFiltering::NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR: return TextureFiltering::LINEAR;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: return TextureFiltering::NEAREST_MIPMAP_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: return TextureFiltering::LINEAR_MIPMAP_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: return TextureFiltering::NEAREST_MIPMAP_LINEAR;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: return TextureFiltering::LINEAR_MIPMAP_LINEAR;
		default: return TextureFiltering::LINEAR;
	}
};

inline TextureWrapping GetTextureWrapping(int32 gltfWrapping)
{
	switch (gltfWrapping) {
		case TINYGLTF_TEXTURE_WRAP_REPEAT: return TextureWrapping::REPEAT;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: return TextureWrapping::CLAMP_TO_EDGE;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return TextureWrapping::MIRRORED_REPEAT;
		default: return TextureWrapping::REPEAT;
	}
};

inline AlphaMode GetAlphaMode(const std::string& gltfAlphaMode)
{
	if (smath::CaseInsensitiveCompare(gltfAlphaMode, "OPAQUE")) {
		return AM_OPAQUE;
	}
	if (smath::CaseInsensitiveCompare(gltfAlphaMode, "MASK")) {
		return AM_MASK;
	}
	if (smath::CaseInsensitiveCompare(gltfAlphaMode, "BLEND")) {
		return AM_BLEND;
	}
	// not defined -> opaque
	return AM_OPAQUE;
}

inline ElementType GetElementType(int32 gltfElementType)
{
	switch (gltfElementType) {
		case TINYGLTF_TYPE_SCALAR: return ElementType::SCALAR;
		case TINYGLTF_TYPE_VEC2: return ElementType::VEC2;
		case TINYGLTF_TYPE_VEC3: return ElementType::VEC3;
		case TINYGLTF_TYPE_VEC4: return ElementType::VEC4;
		case TINYGLTF_TYPE_MAT2: return ElementType::MAT2;
		case TINYGLTF_TYPE_MAT3: return ElementType::MAT3;
		case TINYGLTF_TYPE_MAT4: return ElementType::MAT4;
		default: return ElementType::SCALAR;
	}
}

inline ComponentType GetComponentType(int32 gltfComponentType)
{
	switch (gltfComponentType) {
		case TINYGLTF_COMPONENT_TYPE_BYTE: return ComponentType::CHAR;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return ComponentType::BYTE;
		case TINYGLTF_COMPONENT_TYPE_SHORT: return ComponentType::SHORT;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return ComponentType::USHORT;
		case TINYGLTF_COMPONENT_TYPE_INT: return ComponentType::INT;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return ComponentType::UINT;
		case TINYGLTF_COMPONENT_TYPE_FLOAT: return ComponentType::FLOAT;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE: return ComponentType::DOUBLE;
		default: return ComponentType::FLOAT;
	}
}
} // namespace gltfaux
