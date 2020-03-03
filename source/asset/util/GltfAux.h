#pragma once

#include "core/StringAux.h"

#include <tiny_gltf.h>
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

inline MaterialPod::AlphaMode GetAlphaMode(const std::string& gltfAlphaMode)
{
	if (str::equalInsensitive(gltfAlphaMode, "OPAQUE")) {
		return MaterialPod::OPAQUE_;
	}
	if (str::equalInsensitive(gltfAlphaMode, "MASK")) {
		return MaterialPod::MASK;
	}
	if (str::equalInsensitive(gltfAlphaMode, "BLEND")) {
		return MaterialPod::BLEND;
	}
	// not defined -> opaque
	return MaterialPod::OPAQUE_;
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
