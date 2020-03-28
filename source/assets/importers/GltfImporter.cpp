#include "pch.h"
#include "GltfImporter.h"

#include "assets/AssetImporterManager.h"
#include "core/StringUtl.h"

#include <tinygltf/tiny_gltf.h>

#undef LoadImage

namespace tg = tinygltf;

namespace {

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

inline Material::AlphaMode GetAlphaMode(const std::string& gltfAlphaMode)
{
	if (str::equalInsensitive(gltfAlphaMode, "OPAQUE")) {
		return Material::Opaque;
	}
	if (str::equalInsensitive(gltfAlphaMode, "MASK")) {
		return Material::Mask;
	}
	if (str::equalInsensitive(gltfAlphaMode, "BLEND")) {
		return Material::Blend;
	}
	// not defined -> opaque
	return Material::Opaque;
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

void ExtractIndicesInto(const tg::Model& modelData, int32 accessorIndex, std::vector<uint32>& out)
{
	AccessorDescription desc(modelData, accessorIndex);

	CLOG_ABORT(desc.componentCount != 1, "Found indicies of 2 components in gltf file.");
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

template<typename ComponentType>
void CopyToVertexData_Position(
	std::vector<VertexData>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
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

template<typename ComponentType>
void CopyToVertexData_Normal(
	std::vector<VertexData>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
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

template<typename ComponentType>
void CopyToVertexData_Tangent(
	std::vector<VertexData>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
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

		// normal is ensured to be here
		// if it was calculated, i.e. missing whilst tangents are defined
		// this is an issue with this particular model
		result[i].bitangent = glm::normalize(glm::cross(result[i].normal, result[i].tangent) * handness);
	}
}

template<typename ComponentType>
void CopyToVertexData_TexCoord0(
	std::vector<VertexData>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<double, ComponentType>) {
			result[i].textCoord0[0] = static_cast<float>(data[0]);
			result[i].textCoord0[1] = static_cast<float>(data[1]);
		}
		else {
			static_assert(std::is_same_v<float, ComponentType>);
			result[i].textCoord0[0] = data[0];
			result[i].textCoord0[1] = data[1];
		}

		// mirror to second uv map in case it is missing
		result[i].textCoord1 = result[i].textCoord0;
	}
}

template<typename ComponentType>
void CopyToVertexData_TexCoord1(
	std::vector<VertexData>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<double, ComponentType>) {
			result[i].textCoord1[0] = static_cast<float>(data[0]);
			result[i].textCoord1[1] = static_cast<float>(data[1]);
		}
		else {
			static_assert(std::is_same_v<float, ComponentType>);
			result[i].textCoord1[0] = data[0];
			result[i].textCoord1[1] = data[1];
		}
	}
}

template<size_t VertexElementIndex, typename ComponentType>
void LoadIntoVertexData_Selector(
	std::vector<VertexData>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	if constexpr (VertexElementIndex == 0) {
		CopyToVertexData_Position<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 1) {
		CopyToVertexData_Normal<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 2) {
		CopyToVertexData_Tangent<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 3) {
		CopyToVertexData_TexCoord0<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 4) {
		CopyToVertexData_TexCoord1<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
}

template<size_t VertexElementIndex>
void LoadIntoVertexData(const tg::Model& modelData, int32 accessorIndex, std::vector<VertexData>& out)
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
			LoadIntoVertexData_Selector<VertexElementIndex, float>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE:
			LoadIntoVertexData_Selector<VertexElementIndex, double>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
	}
}

class GltfLoader {
	uri::Uri gltfFilePath;
	uri::Uri filename;
	fs::path systemPath;

	tg::Model model;

	std::vector<PodHandle<Image>> imagePods;
	std::vector<PodHandle<Sampler>> samplerPods;
	std::vector<PodHandle<Material>> materialPods;
	// std::vector<PodHandle<Model>> modelPods;

	void LoadGeometryGroup(
		Model* pod, GeometryGroup& geom, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat);
	void LoadMesh(Model* pod, Mesh& mesh, const tinygltf::Mesh& meshData, const glm::mat4& transformMat);

	void LoadMaterial(Material* pod, size_t index);

	void LoadImages();
	void LoadSamplers();
	void LoadMaterials();

	// CHECK: currently loads default scene as model, could also load all the other scenes in the future
	void LoadModel(Model* pod, int32 sceneIndex);

	// Model specific state
	bool tempModelRequiresDefaultMat{ false };

public:
	GltfLoader(const fs::path& path);

	[[nodiscard]] BasePodHandle Load();
};

GltfLoader::GltfLoader(const fs::path& path)
	: gltfFilePath(path.generic_string())
	, systemPath(path)
{
	tg::TinyGLTF loader;

	std::string err;
	std::string warn;

	filename = path.filename().replace_extension().generic_string();

	const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, gltfFilePath.c_str());

	CLOG_WARN(!warn.empty(), "Gltf Load warning for {}: {}", path, warn.c_str());
	CLOG_ABORT(!err.empty(), "Gltf Load error for {}: {}", path, err.c_str());
}

void GltfLoader::LoadGeometryGroup(
	Model* pod, GeometryGroup& geom, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat)
{

	CLOG_WARN(primitiveData.mode != TINYGLTF_MODE_TRIANGLES, "Unsupported primitive data mode {}", filename);

	// material
	const auto materialIndex = primitiveData.material;

	// If material is -1, we need default material.
	if (materialIndex == -1) {
		// Default material will be placed at last slot.
		geom.materialIndex = static_cast<uint32>(materialPods.size() - 1);
		tempModelRequiresDefaultMat = true;
	}
	else {
		geom.materialIndex = materialIndex;
	}

	auto it = std::find_if(begin(primitiveData.attributes), end(primitiveData.attributes),
		[](auto& pair) { return str::equalInsensitive(pair.first, "POSITION"); });


	size_t vertexCount = model.accessors.at(it->second).count;
	geom.vertices.resize(vertexCount);

	// indexing
	const auto indicesIndex = primitiveData.indices;

	if (indicesIndex != -1) {
		ExtractIndicesInto(model, indicesIndex, geom.indices);
	}
	else {
		geom.indices.resize(vertexCount);
		for (int32 i = 0; i < vertexCount; ++i) {
			geom.indices[i] = i;
		}
	}

	int32 positionsIndex = -1;
	int32 normalsIndex = -1;
	int32 tangentsIndex = -1;
	int32 texcoords0Index = -1;
	int32 texcoords1Index = -1;

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
		else if (str::equalInsensitive(attrName, "TEXCOORD_1")) {
			texcoords1Index = index;
		}
	}

	// load in this order

	// POSITIONS
	if (positionsIndex != -1) {
		LoadIntoVertexData<0>(model, positionsIndex, geom.vertices);
	}
	else {
		LOG_ABORT("Model does not have any positions...");
	}

	// NORMALS
	if (normalsIndex != -1) {
		LoadIntoVertexData<1>(model, normalsIndex, geom.vertices);
	}
	else {
		LOG_DEBUG("Model missing normals, calculating flat normals");

		// calculate missing normals (flat)
		for (int32 i = 0; i < geom.indices.size(); i += 3) {
			// triangle
			auto p0 = geom.vertices[geom.indices[i]].position;
			auto p1 = geom.vertices[geom.indices[i + 1]].position;
			auto p2 = geom.vertices[geom.indices[i + 2]].position;

			glm::vec3 n = glm::cross(p1 - p0, p2 - p0);

			geom.vertices[geom.indices[i]].normal += n;
			geom.vertices[geom.indices[i + 1]].normal += n;
			geom.vertices[geom.indices[i + 2]].normal += n;
		}

		for (auto& v : geom.vertices) {
			v.normal = glm::normalize(v.normal);
		}
	}

	// UV 0
	if (texcoords0Index != -1) {
		LoadIntoVertexData<3>(model, texcoords0Index, geom.vertices);
	}
	else {
		LOG_DEBUG("Model missing first uv map, not handled");
	}

	// UV 1
	if (texcoords1Index != -1) {
		LoadIntoVertexData<4>(model, texcoords1Index, geom.vertices);
	}
	else {
		LOG_DEBUG("Model missing second uv map, mirroring first");
	}

	// TANGENTS, BITANGENTS
	if (tangentsIndex != -1) {
		LoadIntoVertexData<2>(model, tangentsIndex, geom.vertices);
	}
	else {
		if (texcoords0Index != -1 || texcoords1Index != -1) {
			LOG_DEBUG("Model missing tangents, calculating using available uv map");

			for (int32 i = 0; i < geom.indices.size(); i += 3) {
				// triangle
				auto p0 = geom.vertices[geom.indices[i]].position;
				auto p1 = geom.vertices[geom.indices[i + 1]].position;
				auto p2 = geom.vertices[geom.indices[i + 2]].position;

				auto uv0 = texcoords0Index != -1 ? geom.vertices[geom.indices[i]].textCoord0
												 : geom.vertices[geom.indices[i]].textCoord1;
				auto uv1 = texcoords0Index != -1 ? geom.vertices[geom.indices[i + 1]].textCoord0
												 : geom.vertices[geom.indices[i + 1]].textCoord1;
				auto uv2 = texcoords0Index != -1 ? geom.vertices[geom.indices[i + 2]].textCoord0
												 : geom.vertices[geom.indices[i + 2]].textCoord1;

				glm::vec3 edge1 = p1 - p0;
				glm::vec3 edge2 = p2 - p0;
				glm::vec2 deltaUV1 = uv1 - uv0;
				glm::vec2 deltaUV2 = uv2 - uv0;

				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

				glm::vec3 tangent;

				tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
				tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
				tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

				// tangent = glm::normalize(tangent);

				geom.vertices[geom.indices[i]].tangent += tangent;
				geom.vertices[geom.indices[i + 1]].tangent += tangent;
				geom.vertices[geom.indices[i + 2]].tangent += tangent;

				glm::vec3 bitangent;

				bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
				bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
				bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

				// bitangent = glm::normalize(bitangent);

				geom.vertices[geom.indices[i]].bitangent += bitangent;
				geom.vertices[geom.indices[i + 1]].bitangent += bitangent;
				geom.vertices[geom.indices[i + 2]].bitangent += bitangent;
			}

			for (auto& v : geom.vertices) {
				v.tangent = glm::normalize(v.tangent);
				v.bitangent = glm::normalize(v.bitangent);
			}
		}
		else {
			LOG_DEBUG("Model missing tangents (and uv maps), calculating using hack");

			for (auto& v : geom.vertices) {
				const auto c1 = glm::cross(v.normal, glm::vec3(0.0, 0.0, 1.0));
				const auto c2 = glm::cross(v.normal, glm::vec3(0.0, 1.0, 0.0));

				v.tangent = glm::length2(c1) > glm::length2(c2) ? glm::normalize(c1) : glm::normalize(c2);
				v.bitangent = glm::normalize(glm::cross(v.normal, glm::vec3(v.tangent)));
			}
		}
	}

	// Bake transform
	const auto invTransMat = glm::transpose(glm::inverse(glm::mat3(transformMat)));
	for (auto& v : geom.vertices) {

		v.position = transformMat * glm::vec4(v.position, 1.f);

		v.normal = invTransMat * v.normal;
		v.tangent = invTransMat * v.tangent;
		v.bitangent = invTransMat * v.bitangent;

		pod->bbox.min = glm::min(pod->bbox.min, v.position);
		pod->bbox.max = glm::max(pod->bbox.max, v.position);
	}
}

void GltfLoader::LoadMesh(Model* pod, Mesh& mesh, const tinygltf::Mesh& meshData, const glm::mat4& transformMat)
{
	mesh.geometryGroups.resize(meshData.primitives.size());

	// primitives
	for (int32 i = 0; i < mesh.geometryGroups.size(); ++i) {
		const auto geomName = "geom_group" + std::to_string(i);

		auto& primitiveData = meshData.primitives.at(i);

		// if one of the geometry groups fails to load
		LoadGeometryGroup(pod, mesh.geometryGroups[i], primitiveData, transformMat);
	}
}

void GltfLoader::LoadMaterial(Material* pod, size_t index)
{
	auto& data = model.materials.at(index);

	// factors
	auto bFactor = data.pbrMetallicRoughness.baseColorFactor;
	pod->baseColorFactor = { bFactor[0], bFactor[1], bFactor[2], bFactor[3] };
	pod->metallicFactor = static_cast<float>(data.pbrMetallicRoughness.metallicFactor);
	pod->roughnessFactor = static_cast<float>(data.pbrMetallicRoughness.roughnessFactor);
	auto eFactor = data.emissiveFactor;
	pod->emissiveFactor = { eFactor[0], eFactor[1], eFactor[2] };

	// scales/strenghts
	pod->normalScale = static_cast<float>(data.normalTexture.scale);
	pod->occlusionStrength = static_cast<float>(data.occlusionTexture.strength);

	// alpha
	pod->alphaMode = GetAlphaMode(data.alphaMode);

	pod->alphaCutoff = static_cast<float>(data.alphaCutoff);
	// doublesided-ness
	pod->doubleSided = data.doubleSided;

	auto fillMatTexture = [&](auto textureInfo, PodHandle<Sampler>& sampler, PodHandle<Image>& image, int32& uvIndex) {
		if (textureInfo.index != -1) {

			auto texture = model.textures.at(textureInfo.index);

			const auto imageIndex = texture.source;
			CLOG_ABORT(imageIndex == -1, "This model is unsafe to use");

			image = imagePods.at(imageIndex);

			const auto samplerIndex = texture.sampler;
			if (samplerIndex != -1) {
				sampler = samplerPods.at(samplerIndex);
			} // else default

			uvIndex = textureInfo.texCoord;
		}
	};

	// samplers
	auto& baseColorTextureInfo = data.pbrMetallicRoughness.baseColorTexture;
	fillMatTexture(baseColorTextureInfo, pod->baseColorSampler, pod->baseColorImage, pod->baseColorUvIndex);

	auto& emissiveTextureInfo = data.emissiveTexture;
	fillMatTexture(emissiveTextureInfo, pod->emissiveSampler, pod->emissiveImage, pod->emissiveUvIndex);

	auto& normalTextureInfo = data.normalTexture;
	fillMatTexture(normalTextureInfo, pod->normalSampler, pod->normalImage, pod->normalUvIndex);

	auto& metallicRougnessTextureInfo = data.pbrMetallicRoughness.metallicRoughnessTexture;
	fillMatTexture(metallicRougnessTextureInfo, pod->metallicRoughnessSampler, pod->metallicRoughnessImage,
		pod->metallicRoughnessUvIndex);

	auto& occlusionTextureInfo = data.occlusionTexture;
	fillMatTexture(occlusionTextureInfo, pod->occlusionSampler, pod->occlusionImage, pod->occlusionUvIndex);
}

void GltfLoader::LoadImages()
{
	// CHECK: embedded images not supported currently
	for (auto& img : model.images) {
		fs::path imgPath = systemPath.remove_filename() / img.uri;
		imagePods.push_back(ImporterManager->ImportRequest<Image>(imgPath));
	}
}

void GltfLoader::LoadSamplers()
{
	int32 samplerIndex = 0;
	for (auto& sampler : model.samplers) {

		nlohmann::json data;
		data["sampler"] = samplerIndex;
		auto samplerPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = sampler.name.empty() ? filename + "_Sampler_" + std::to_string(samplerIndex) : sampler.name;

		auto& [handle, pod] = ImporterManager->CreateEntry<Sampler>(samplerPath, name);

		pod->minFilter = GetTextureFiltering(sampler.minFilter);
		pod->magFilter = GetTextureFiltering(sampler.magFilter);

		// Based on opengl convections min filter contains the mip map filtering information
		pod->mipmapFilter = GetMipmapFiltering(sampler.minFilter);

		pod->wrapU = GetTextureWrapping(sampler.wrapS);
		pod->wrapV = GetTextureWrapping(sampler.wrapT);
		pod->wrapW = GetTextureWrapping(sampler.wrapR);

		samplerPods.push_back(handle);
		samplerIndex++;
	}
}

void GltfLoader::LoadMaterials()
{
	int32 matIndex = 0;
	for (auto& mat : model.materials) {

		nlohmann::json data;
		data["material"] = matIndex;
		auto matPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = mat.name.empty() ? filename + "_Mat_" + std::to_string(matIndex) : mat.name;

		auto& [handle, pod] = ImporterManager->CreateEntry<Material>(matPath, name);

		LoadMaterial(pod, matIndex);

		materialPods.emplace_back(handle);
		matIndex++;
	}
	// auto& [handle, pod]
	//	= AssetImporterManager::CreateEntry<Model>(gltfFilePath, std::string(uri::GetFilenameNoExt(gltfFilePath)));

	materialPods.push_back({}); // bleh
}

void GltfLoader::LoadModel(Model* pod, int32 sceneIndex)
{
	int32 scene = sceneIndex >= 0 ? sceneIndex : 0;

	auto& defaultScene = model.scenes.at(scene);

	tempModelRequiresDefaultMat = false;

	std::function<bool(const std::vector<int>&, glm::mat4)> RecurseChildren;
	RecurseChildren = [&](const std::vector<int>& childrenIndices, glm::mat4 parentTransformMat) {
		for (auto& nodeIndex : childrenIndices) {
			auto& childNode = model.nodes.at(nodeIndex);

			glm::mat4 localTransformMat = glm::mat4(1.f);

			// When matrix is defined, it must be decomposable to TRS.
			if (!childNode.matrix.empty()) {
				for (int32 row = 0; row < 4; ++row) {
					for (int32 column = 0; column < 4; ++column) {
						localTransformMat[row][column] = static_cast<float>(childNode.matrix[column + 4llu * row]);
					}
				}
			}
			else {
				glm::vec3 translation = glm::vec3(0.f);
				glm::quat orientation = { 1.f, 0.f, 0.f, 0.f };
				glm::vec3 scale = glm::vec3(1.f);

				if (!childNode.translation.empty()) {
					translation[0] = static_cast<float>(childNode.translation[0]);
					translation[1] = static_cast<float>(childNode.translation[1]);
					translation[2] = static_cast<float>(childNode.translation[2]);
				}

				if (!childNode.rotation.empty()) {
					orientation[0] = static_cast<float>(childNode.rotation[0]);
					orientation[1] = static_cast<float>(childNode.rotation[1]);
					orientation[2] = static_cast<float>(childNode.rotation[2]);
					orientation[3] = static_cast<float>(childNode.rotation[3]);
				}

				if (!childNode.scale.empty()) {
					scale[0] = static_cast<float>(childNode.scale[0]);
					scale[1] = static_cast<float>(childNode.scale[1]);
					scale[2] = static_cast<float>(childNode.scale[2]);
				}

				localTransformMat = math::transformMat(scale, orientation, translation);
			}

			localTransformMat = parentTransformMat * localTransformMat;

			// load mesh if exists
			if (childNode.mesh != -1) {
				auto& gltfMesh = model.meshes.at(childNode.mesh);

				Mesh mesh;
				LoadMesh(pod, mesh, gltfMesh, localTransformMat);
				pod->meshes.emplace_back(mesh);
			}

			// load child's children
			if (!childNode.children.empty()) {
				if (!RecurseChildren(childNode.children, localTransformMat)) {
					return false;
				}
			}
		}
		return true;
	};

	RecurseChildren(defaultScene.nodes, glm::mat4(1.f));


	pod->materials = materialPods;

	if (!tempModelRequiresDefaultMat) {
		pod->materials.pop_back();
	}
}

BasePodHandle GltfLoader::Load()
{
	auto& [handle, pod]
		= ImporterManager->CreateEntry<Model>(gltfFilePath, std::string(uri::GetFilenameNoExt(gltfFilePath)));

	LoadImages();
	LoadSamplers();
	LoadMaterials();

	LoadModel(pod, model.defaultScene);

	return handle;
}
} // namespace

BasePodHandle GltfImporter::Import(const fs::path& path)
{
	GltfLoader loader{ path };
	return loader.Load();
}
