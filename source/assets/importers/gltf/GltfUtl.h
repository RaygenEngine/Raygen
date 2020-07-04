#pragma once
#include "assets/pods/Animation.h"
#include "assets/pods/MaterialInstance.h"
#include "assets/pods/Mesh.h"
#include "assets/pods/Sampler.h"
#include "core/StringUtl.h"

#include <tinygltf/tiny_gltf.h>

namespace tg = tinygltf;

namespace gltfutl {
// TODO: remove once the integration of the material slots is complete
struct GeometryGroup {
	std::vector<uint32> indices{};
	std::vector<Vertex> vertices{};

	uint32 materialIndex;
};

inline TextureFiltering GetTextureFiltering(int32 gltfFiltering)
{
	switch (gltfFiltering) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: return TextureFiltering::Nearest;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: return TextureFiltering::Linear;
		default: return TextureFiltering::Linear;
	}
}

inline MipmapFiltering GetMipmapFiltering(int32 gltfFiltering)
{
	switch (gltfFiltering) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR: return MipmapFiltering::NoMipmap;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: return MipmapFiltering::Nearest;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: return MipmapFiltering::Linear;
		default: return MipmapFiltering::Linear;
	}
}

inline TextureWrapping GetTextureWrapping(int32 gltfWrapping)
{
	switch (gltfWrapping) {
		case TINYGLTF_TEXTURE_WRAP_REPEAT: return TextureWrapping::Repeat;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: return TextureWrapping::ClampToEdge;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return TextureWrapping::MirroredRepeat;
		default: return TextureWrapping::Repeat;
	}
};

inline AnimationPath GetAnimationPath(const std::string& gltfAnimationPath)
{
	if (str::equalInsensitive(gltfAnimationPath, "translation")) {
		return AnimationPath::Translation;
	}
	if (str::equalInsensitive(gltfAnimationPath, "rotation")) {
		return AnimationPath::Rotation;
	}
	if (str::equalInsensitive(gltfAnimationPath, "scale")) {
		return AnimationPath::Scale;
	}
	if (str::equalInsensitive(gltfAnimationPath, "weights")) {
		return AnimationPath::Weights;
	}
	// not defined -> translation, CHECK: or should abort?
	return AnimationPath::Translation;
}

inline InterpolationMethod GetInterpolationMethod(const std::string& gltfInterpolationMethod)
{
	if (str::equalInsensitive(gltfInterpolationMethod, "LINEAR")) {
		return InterpolationMethod::Linear;
	}
	if (str::equalInsensitive(gltfInterpolationMethod, "STEP")) {
		return InterpolationMethod::Step;
	}
	if (str::equalInsensitive(gltfInterpolationMethod, "CUBICSPLINE")) {
		return InterpolationMethod::Cubicspline;
	}
	// not defined -> linear
	return InterpolationMethod::Linear;
}

// TODO: inline float GetNormalizedIntToFloat()
// accessor.normalized = true
// 5120 (BYTE)				f = max(c / 127.0, -1.0)
// 5121 (UNSIGNED_BYTE)		f = c / 255.0
// 5122 (SHORT)				f = max(c / 32767.0, -1.0)
// 5123 (UNSIGNED_SHORT)	f = c / 65535.0

enum class MaterialAlphaMode
{
	Opaque,
	Mask,
	Blend
};

inline MaterialAlphaMode GetAlphaMode(const std::string& gltfAlphaMode)
{
	if (str::equalInsensitive(gltfAlphaMode, "OPAQUE")) {
		return MaterialAlphaMode::Opaque;
	}
	if (str::equalInsensitive(gltfAlphaMode, "MASK")) {
		return MaterialAlphaMode::Mask;
	}
	if (str::equalInsensitive(gltfAlphaMode, "BLEND")) {
		return MaterialAlphaMode::Blend;
	}
	// not defined -> opaque
	return MaterialAlphaMode::Opaque;
}

struct AccessorDescription {
	//
	// Actual example of a possible complex gltf buffer:
	//                                              |     STRIDE  |
	// [{vertexIndexes} * 1000] [{normals} * 1000] [{uv0, position} * 1000]
	//													  ^ beginPtr for Position.
	//

	size_t elementCount;   // How many elements there are to read
	size_t componentCount; // How many components of type ComponentType there are to each element.

	size_t strideByteOffset; // The number of bytes to move in the buffer after each read to get the next element.
							 // This may be more bytes than the actual sizeof(ComponentType) * componentCount
							 // if the data is strided.

	byte* beginPtr; // Pointer to the first byte we care about.
					// This may not be the actual start of the buffer of the binary file.

	uint32 componentType; // this particular model's underlying buffer type to read as.

	AccessorDescription(const tg::Model& modelData, int32 accessorIndex)
	{
		size_t beginByteOffset;
		const tinygltf::Accessor& accessor = modelData.accessors.at(accessorIndex);
		const tinygltf::BufferView& bufferView = modelData.bufferViews.at(accessor.bufferView);
		const tinygltf::Buffer& gltfBuffer = modelData.buffers.at(bufferView.buffer);


		componentType = accessor.componentType;
		elementCount = accessor.count;
		beginByteOffset = accessor.byteOffset + bufferView.byteOffset;
		strideByteOffset = accessor.ByteStride(bufferView);
		componentCount = tg::GetNumComponentsInType(accessor.type);
		beginPtr = const_cast<byte*>(&gltfBuffer.data[beginByteOffset]);
	}
};


// Uint16 specialization. Expects componentCount == 1.
template<typename T>
void CopyToVector(std::vector<uint32>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	static_assert(std::is_integral_v<T>, "This is not an integer type");

	for (uint32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		T* data = reinterpret_cast<T*>(elementPtr);
		result[i] = *data;
	}
}

// Expects float type, componentCount == 1
inline void CopyToFloatVector(std::vector<float>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (uint32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		float* data = reinterpret_cast<float*>(elementPtr);
		result[i] = *data;
	}
}

inline void ExtractMatrices4Into(const tg::Model& modelData, int32 accessorIndex, std::vector<glm::mat4>& out)
{
	AccessorDescription desc(modelData, accessorIndex);
	out.resize(desc.elementCount);

	switch (desc.componentType) {
		case TINYGLTF_COMPONENT_TYPE_BYTE:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		case TINYGLTF_COMPONENT_TYPE_SHORT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		case TINYGLTF_COMPONENT_TYPE_INT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		case TINYGLTF_COMPONENT_TYPE_DOUBLE: LOG_ABORT("Incorrect buffers, debug model...");
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			for (int32 i = 0; i < desc.elementCount; ++i) {
				byte* elementPtr = &desc.beginPtr[desc.strideByteOffset * i];
				float* data = reinterpret_cast<float*>(elementPtr);

				memcpy(&out[i], data, sizeof(glm::mat4));
			}
			return;
	}
}

inline void ExtractIndicesInto(const tg::Model& modelData, int32 accessorIndex, std::vector<uint32>& out)
{
	AccessorDescription desc(modelData, accessorIndex);

	CLOG_ABORT(desc.componentCount != 1, "Found indices of 2 components in gltf file.");
	out.resize(desc.elementCount);

	switch (desc.componentType) {
			// Conversions from signed to unsigned types are "implementation defined".
			// This code assumes the implementation will not do any bit arethmitic from signed x to unsigned x.

		case TINYGLTF_COMPONENT_TYPE_BYTE:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
			CopyToVector<unsigned char>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_SHORT: {
			CopyToVector<short>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
			CopyToVector<unsigned short>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_INT: {
			CopyToVector<int>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
			CopyToVector<uint32>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
		case TINYGLTF_COMPONENT_TYPE_DOUBLE: return;
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_Position(
	std::vector<VertexT>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<double, ComponentType>) {
			result[i].position[0] = static_cast<float>(data[0]);
			result[i].position[1] = static_cast<float>(data[1]);
			result[i].position[2] = static_cast<float>(data[2]);
		}
		else {
			static_assert(std::is_same_v<float, ComponentType>);
			result[i].position[0] = data[0];
			result[i].position[1] = data[1];
			result[i].position[2] = data[2];
		}
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_Normal(std::vector<VertexT>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<double, ComponentType>) {
			result[i].normal[0] = static_cast<float>(data[0]);
			result[i].normal[1] = static_cast<float>(data[1]);
			result[i].normal[2] = static_cast<float>(data[2]);
		}
		else {
			static_assert(std::is_same_v<float, ComponentType>);
			result[i].normal[0] = data[0];
			result[i].normal[1] = data[1];
			result[i].normal[2] = data[2];
		}
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_Tangent(
	std::vector<VertexT>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		float handness = 1.f;

		if constexpr (std::is_same_v<double, ComponentType>) {
			result[i].tangent[0] = static_cast<float>(data[0]);
			result[i].tangent[1] = static_cast<float>(data[1]);
			result[i].tangent[2] = static_cast<float>(data[2]);
			handness = static_cast<float>(data[3]);
		}
		else {
			static_assert(std::is_same_v<float, ComponentType>);
			result[i].tangent[0] = data[0];
			result[i].tangent[1] = data[1];
			result[i].tangent[2] = data[2];
			handness = data[3];
		}

		result[i].tangent *= handness;
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_TexCoord0(
	std::vector<VertexT>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<double, ComponentType>) {
			result[i].uv[0] = static_cast<float>(data[0]);
			result[i].uv[1] = static_cast<float>(data[1]);
		}
		else {
			static_assert(std::is_same_v<float, ComponentType>);
			result[i].uv[0] = data[0];
			result[i].uv[1] = data[1];
		}
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_Joint(std::vector<VertexT>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<uint16, ComponentType>) {
			result[i].joint[0] = static_cast<uint16>(data[0]);
			result[i].joint[1] = static_cast<uint16>(data[1]);
			result[i].joint[2] = static_cast<uint16>(data[2]);
			result[i].joint[3] = static_cast<uint16>(data[3]);
		}
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_Weight(std::vector<VertexT>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<float, ComponentType>) {
			result[i].weight[0] = static_cast<float>(data[0]);
			result[i].weight[1] = static_cast<float>(data[1]);
			result[i].weight[2] = static_cast<float>(data[2]);
			result[i].weight[3] = static_cast<float>(data[3]);
		}
	}
}

template<typename VertexT, size_t VertexElementIndex, typename ComponentType>
void LoadIntoVertexData_Selector(
	std::vector<VertexT>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	if constexpr (VertexElementIndex == 0) {
		CopyToVertexData_Position<VertexT, ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 1) {
		CopyToVertexData_Normal<VertexT, ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 2) {
		CopyToVertexData_Tangent<VertexT, ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 3) {
		CopyToVertexData_TexCoord0<VertexT, ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 4) {
		CopyToVertexData_Joint<VertexT, ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 5) {
		CopyToVertexData_Weight<VertexT, ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
}

template<typename VertexT, size_t VertexElementIndex>
void LoadIntoVertexData(const tg::Model& modelData, int32 accessorIndex, std::vector<VertexT>& out)
{
	AccessorDescription desc(modelData, accessorIndex);

	switch (desc.componentType) {
		case TINYGLTF_COMPONENT_TYPE_BYTE:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		case TINYGLTF_COMPONENT_TYPE_SHORT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		case TINYGLTF_COMPONENT_TYPE_INT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: LOG_ABORT("Incorrect buffers, debug model...");

		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			LoadIntoVertexData_Selector<VertexT, VertexElementIndex, float>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE:
			LoadIntoVertexData_Selector<VertexT, VertexElementIndex, double>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
	}
}
} // namespace gltfutl
