#pragma once
#include "assets/importers/gltf/GltfCache.h"
#include "assets/pods/Animation.h"
#include "assets/pods/MaterialInstance.h"
#include "assets/pods/Mesh.h"
#include "assets/pods/Sampler.h"
#include "core/StringUtl.h"
#include "engine/Logger.h"

namespace tg = tinygltf;

namespace gltfutl {
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

inline float NormalizedIntToFloat(int8 c)
{
	return glm::max(c / 127.f, -1.f);
}

inline float NormalizedIntToFloat(uint8 c)
{
	return c / 255.f;
}

inline float NormalizedIntToFloat(int16 c)
{
	return glm::max(c / 32767.f, -1.f);
}

inline float NormalizedIntToFloat(uint16 c)
{
	return c / 65535.f;
}

inline float NormalizedIntToFloat(float c)
{
	return c;
}

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

	bool normalized;

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

		normalized = accessor.normalized;
	}
};


// Uint16 specialization. Expects componentCount == 1.
template<typename T>
void CopyIndicesVector(uint32* result, byte* beginPtr, size_t perElementOffset, size_t elementCount, uint32 offset)
{
	static_assert(std::is_integral_v<T>, "This is not an integer type");

	for (uint32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		T* data = reinterpret_cast<T*>(elementPtr);
		result[i] = static_cast<uint32>(*data) + offset;
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

inline void ExtractIndicesInto(const tg::Model& modelData, AccessorDescription& inDesc, uint32* out, uint32 indexOffset)
{
	switch (inDesc.componentType) {
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
		case TINYGLTF_COMPONENT_TYPE_DOUBLE:
			LOG_ABORT("Incorrect buffers, debug model...");

			// Conversions from signed to unsigned types are "implementation defined".
			// This code assumes the implementation will not do any bit arethmitic from signed x to unsigned x.

		case TINYGLTF_COMPONENT_TYPE_BYTE: {
			CopyIndicesVector<int8>(out, inDesc.beginPtr, inDesc.strideByteOffset, inDesc.elementCount, indexOffset);
			return;
		}

		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
			CopyIndicesVector<uint8>(out, inDesc.beginPtr, inDesc.strideByteOffset, inDesc.elementCount, indexOffset);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_SHORT: {
			CopyIndicesVector<int16>(out, inDesc.beginPtr, inDesc.strideByteOffset, inDesc.elementCount, indexOffset);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
			CopyIndicesVector<uint16>(out, inDesc.beginPtr, inDesc.strideByteOffset, inDesc.elementCount, indexOffset);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_INT: {
			CopyIndicesVector<int32>(out, inDesc.beginPtr, inDesc.strideByteOffset, inDesc.elementCount, indexOffset);
			return;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
			CopyIndicesVector<uint32>(out, inDesc.beginPtr, inDesc.strideByteOffset, inDesc.elementCount, indexOffset);
			return;
		}
	}
}

template<typename VertexT>
void CopyToVertexData_Position(VertexT* result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		float* data = reinterpret_cast<float*>(elementPtr);

		result[i].position[0] = data[0];
		result[i].position[1] = data[1];
		result[i].position[2] = data[2];
	}
}

template<typename VertexT>
void CopyToVertexData_Normal(VertexT* result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		float* data = reinterpret_cast<float*>(elementPtr);

		result[i].normal[0] = data[0];
		result[i].normal[1] = data[1];
		result[i].normal[2] = data[2];
	}
}

template<typename VertexT>
void CopyToVertexData_Tangent(VertexT* result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];

		float* data = reinterpret_cast<float*>(elementPtr);

		result[i].tangent[0] = data[0];
		result[i].tangent[1] = data[1];
		result[i].tangent[2] = data[2];

		// data[3] == handness
		result[i].tangent *= data[3];
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_TexCoord0(VertexT* result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		result[i].uv[0] = NormalizedIntToFloat(data[0]);
		result[i].uv[1] = NormalizedIntToFloat(data[1]);
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_Joint(VertexT* result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		result[i].joint[0] = static_cast<uint16>(data[0]);
		result[i].joint[1] = static_cast<uint16>(data[1]);
		result[i].joint[2] = static_cast<uint16>(data[2]);
		result[i].joint[3] = static_cast<uint16>(data[3]);
	}
}

template<typename VertexT, typename ComponentType>
void CopyToVertexData_Weight(VertexT* result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		result[i].weight[0] = NormalizedIntToFloat(data[0]);
		result[i].weight[1] = NormalizedIntToFloat(data[1]);
		result[i].weight[2] = NormalizedIntToFloat(data[2]);
		result[i].weight[3] = NormalizedIntToFloat(data[3]);
	}
}

template<typename VertexT, size_t VertexElementIndex, typename ComponentType>
void LoadIntoVertexData_Selector(VertexT* result, byte* beginPtr, size_t perElementOffset, //
	size_t elementCount)
{
	if constexpr (VertexElementIndex == 0) {
		CopyToVertexData_Position<VertexT>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 1) {
		CopyToVertexData_Normal<VertexT>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 2) {
		CopyToVertexData_Tangent<VertexT>(result, beginPtr, perElementOffset, elementCount);
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
void LoadIntoVertexData(const tg::Model& modelData, int32 accessorIndex, VertexT* out)
{
	AccessorDescription desc(modelData, accessorIndex);

	switch (desc.componentType) {

		case TINYGLTF_COMPONENT_TYPE_INT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		case TINYGLTF_COMPONENT_TYPE_DOUBLE: LOG_ABORT("Incorrect buffers, debug model...");

		case TINYGLTF_COMPONENT_TYPE_BYTE:
			LoadIntoVertexData_Selector<VertexT, VertexElementIndex, int8>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;

		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			LoadIntoVertexData_Selector<VertexT, VertexElementIndex, uint8>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;

		case TINYGLTF_COMPONENT_TYPE_SHORT:
			LoadIntoVertexData_Selector<VertexT, VertexElementIndex, int16>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;

		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			LoadIntoVertexData_Selector<VertexT, VertexElementIndex, uint16>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;

		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			LoadIntoVertexData_Selector<VertexT, VertexElementIndex, float>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
	}
}

// returns latest begin and size of slot
template<typename GeometrySlotT, typename VertexT>
inline std::pair<size_t, size_t> LoadBasicDataIntoGeometrySlot(GeometrySlotT& geom, GltfCache& cache, //
	const tinygltf::Primitive& primitiveData)
{
	auto it = std::find_if(begin(primitiveData.attributes), end(primitiveData.attributes),
		[](auto& pair) { return str::equalInsensitive(pair.first, "POSITION"); });

	size_t prevEnd = geom.vertices.size();
	size_t newCount = cache.gltfData.accessors.at(it->second).count;
	size_t totalSize = prevEnd + newCount;

	geom.vertices.resize(totalSize);

	// indexing
	const auto indicesIndex = primitiveData.indices;

	size_t prevIndicesEnd;
	size_t newIndicesCount;
	size_t totalIndicesSize;

	if (indicesIndex == -1) {
		prevIndicesEnd = geom.indices.size();
		newIndicesCount = newCount;
		totalIndicesSize = prevIndicesEnd + newIndicesCount;
		geom.indices.resize(totalIndicesSize);
		for (size_t i = prevEnd; i < totalSize; ++i) {
			geom.indices[i] = static_cast<uint32>(i);
		}
	}
	else {

		AccessorDescription desc(cache.gltfData, indicesIndex);

		CLOG_ABORT(desc.componentCount != 1, "Found indices of 2 components in gltf file.");
		prevIndicesEnd = geom.indices.size();
		newIndicesCount = desc.elementCount;
		totalIndicesSize = prevIndicesEnd + newIndicesCount;
		geom.indices.resize(totalIndicesSize);

		ExtractIndicesInto(cache.gltfData, desc, geom.indices.data() + prevIndicesEnd, prevEnd);
	}

	int32 positionsIndex = -1;
	int32 normalsIndex = -1;
	int32 tangentsIndex = -1;
	int32 texcoords0Index = -1;

	// attributes
	for (auto& attribute : primitiveData.attributes) {
		const auto& attrName = attribute.first;
		int32 index = attribute.second;

		if (str::equalInsensitive(attrName, "POSITION")) {
			positionsIndex = index;
		}
		else if (str::equalInsensitive(attrName, "NORMAL")) {
			normalsIndex = index;
		}
		else if (str::equalInsensitive(attrName, "TANGENT")) {
			tangentsIndex = index;
		}
		else if (str::equalInsensitive(attrName, "TEXCOORD_0")) {
			texcoords0Index = index;
		}
	}

	// load in this order

	// POSITIONS
	if (positionsIndex != -1) {
		LoadIntoVertexData<VertexT, 0>(cache.gltfData, positionsIndex, geom.vertices.data() + prevEnd);
	}
	else {
		LOG_ABORT("Model does not have any positions...");
	}

	// NORMALS
	if (normalsIndex != -1) {
		LoadIntoVertexData<VertexT, 1>(cache.gltfData, normalsIndex, geom.vertices.data() + prevEnd);
	}
	else {
		LOG_DEBUG("Model missing normals, calculating flat normals");

		// calculate missing normals (flat)
		for (size_t i = prevIndicesEnd; i < totalIndicesSize; i += 3) {
			// triangle
			auto p0 = geom.vertices[geom.indices[i]].position;
			auto p1 = geom.vertices[geom.indices[i + 1llu]].position;
			auto p2 = geom.vertices[geom.indices[i + 2llu]].position;

			glm::vec3 n = glm::cross(p1 - p0, p2 - p0);

			geom.vertices[geom.indices[i]].normal += n;
			geom.vertices[geom.indices[i + 1llu]].normal += n;
			geom.vertices[geom.indices[i + 2llu]].normal += n;
		}

		for (size_t i = prevEnd; i < totalSize; ++i) {
			geom.vertices[i].normal = glm::normalize(geom.vertices[i].normal);
		}
	}

	// UV 0
	if (texcoords0Index != -1) {
		LoadIntoVertexData<VertexT, 3>(cache.gltfData, texcoords0Index, geom.vertices.data() + prevEnd);
	}
	else {
		LOG_DEBUG("Model missing first uv map, not handled");
	}

	// TANGENTS, BITANGENTS
	if (tangentsIndex != -1) {
		LoadIntoVertexData<VertexT, 2>(cache.gltfData, tangentsIndex, geom.vertices.data() + prevEnd);
	}
	else {
		if (texcoords0Index != -1) {
			LOG_DEBUG("Model missing tangents, calculating using available uv map");

			for (size_t i = prevIndicesEnd; i < totalIndicesSize; i += 3) {
				// triangle
				auto p0 = geom.vertices[geom.indices[i]].position;
				auto p1 = geom.vertices[geom.indices[i + 1llu]].position;
				auto p2 = geom.vertices[geom.indices[i + 2llu]].position;

				auto uv0 = geom.vertices[geom.indices[i]].uv;
				auto uv1 = geom.vertices[geom.indices[i + 1llu]].uv;
				auto uv2 = geom.vertices[geom.indices[i + 2llu]].uv;

				glm::vec3 edge1 = p1 - p0;
				glm::vec3 edge2 = p2 - p0;
				glm::vec2 deltaUV1 = uv1 - uv0;
				glm::vec2 deltaUV2 = uv2 - uv0;

				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

				glm::vec3 tangent;

				tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
				tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
				tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

				geom.vertices[geom.indices[i]].tangent += tangent;
				geom.vertices[geom.indices[i + 1llu]].tangent += tangent;
				geom.vertices[geom.indices[i + 2llu]].tangent += tangent;
			}

			// CHECK: handness
			for (size_t i = prevEnd; i < totalSize; ++i) {
				geom.vertices[i].tangent = glm::normalize(geom.vertices[i].tangent);
			}
		}
		else {
			LOG_DEBUG("Model missing tangents (and uv maps), calculating using hack");

			for (size_t i = prevEnd; i < totalSize; ++i) {
				const auto c1 = glm::cross(geom.vertices[i].normal, glm::vec3(0.0, 0.0, 1.0));
				const auto c2 = glm::cross(geom.vertices[i].normal, glm::vec3(0.0, 1.0, 0.0));

				geom.vertices[i].tangent = glm::normalize(glm::length2(c1) > glm::length2(c2) ? c1 : c2);
			}
		}
	}

	return { prevEnd, totalSize };
}
} // namespace gltfutl
