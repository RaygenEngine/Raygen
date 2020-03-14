#include "pch.h"
#include "asset/importers/GltfImporter.h"

#include "asset/AssetManager.h"
#include "asset/util/GltfAux.h"

#include <tiny_gltf.h>

#undef LoadImage

namespace tg = tinygltf;

namespace {

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

	ComponentType componentType; // this particular model's underlying buffer type to read as.

	AccessorDescription(const tg::Model& modelData, int32 accessorIndex)
	{
		size_t beginByteOffset;
		const tinygltf::Accessor& accessor = modelData.accessors.at(accessorIndex);
		const tinygltf::BufferView& bufferView = modelData.bufferViews.at(accessor.bufferView);
		const tinygltf::Buffer& gltfBuffer = modelData.buffers.at(bufferView.buffer);


		componentType = gltfaux::GetComponentType(accessor.componentType);
		elementCount = accessor.count;
		beginByteOffset = accessor.byteOffset + bufferView.byteOffset;
		strideByteOffset = accessor.ByteStride(bufferView);
		componentCount = utl::ElementComponentCount(gltfaux::GetElementType(accessor.type));
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

		case ComponentType::CHAR:
		case ComponentType::BYTE: {
			CopyToVector<unsigned char>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case ComponentType::SHORT: {
			CopyToVector<short>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case ComponentType::USHORT: {
			CopyToVector<unsigned short>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case ComponentType::INT: {
			CopyToVector<int>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case ComponentType::UINT: {
			CopyToVector<uint32>(out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		}
		case ComponentType::FLOAT:
		case ComponentType::DOUBLE: return;
	}
}

template<typename ComponentType>
void CopyToVertexData_Position(
	std::vector<VertexData>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount)
{
	for (int32 i = 0; i < elementCount; ++i) {
		byte* elementPtr = &beginPtr[perElementOffset * i];
		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

		if constexpr (std::is_same_v<double, ComponentType>) { // NOLINT
			result[i].position[0] = static_cast<float>(data[0]);
			result[i].position[1] = static_cast<float>(data[1]);
			result[i].position[2] = static_cast<float>(data[2]);
		}
		else { // NOLINT
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

		if constexpr (std::is_same_v<double, ComponentType>) { // NOLINT
			result[i].normal[0] = static_cast<float>(data[0]);
			result[i].normal[1] = static_cast<float>(data[1]);
			result[i].normal[2] = static_cast<float>(data[2]);
		}
		else { // NOLINT
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

		if constexpr (std::is_same_v<double, ComponentType>) { // NOLINT
			result[i].tangent[0] = static_cast<float>(data[0]);
			result[i].tangent[1] = static_cast<float>(data[1]);
			result[i].tangent[2] = static_cast<float>(data[2]);
			handness = static_cast<float>(data[3]);
		}
		else { // NOLINT
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

		if constexpr (std::is_same_v<double, ComponentType>) { // NOLINT
			result[i].textCoord0[0] = static_cast<float>(data[0]);
			result[i].textCoord0[1] = static_cast<float>(data[1]);
		}
		else { // NOLINT
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

		if constexpr (std::is_same_v<double, ComponentType>) { // NOLINT
			result[i].textCoord1[0] = static_cast<float>(data[0]);
			result[i].textCoord1[1] = static_cast<float>(data[1]);
		}
		else { // NOLINT
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
	if constexpr (VertexElementIndex == 0) { // NOLINT
		CopyToVertexData_Position<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 1) { // NOLINT
		CopyToVertexData_Normal<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 2) { // NOLINT
		CopyToVertexData_Tangent<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 3) { // NOLINT
		CopyToVertexData_TexCoord0<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
	else if constexpr (VertexElementIndex == 4) { // NOLINT
		CopyToVertexData_TexCoord1<ComponentType>(result, beginPtr, perElementOffset, elementCount);
	}
}

template<size_t VertexElementIndex>
void LoadIntoVertexData(const tg::Model& modelData, int32 accessorIndex, std::vector<VertexData>& out)
{
	AccessorDescription desc(modelData, accessorIndex);

	switch (desc.componentType) {
		case ComponentType::CHAR:
		case ComponentType::BYTE:
		case ComponentType::SHORT:
		case ComponentType::USHORT:
		case ComponentType::INT:
		case ComponentType::UINT: LOG_ABORT("Incorrect buffers, debug model...");
		case ComponentType::FLOAT:
			LoadIntoVertexData_Selector<VertexElementIndex, float>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
		case ComponentType::DOUBLE:
			LoadIntoVertexData_Selector<VertexElementIndex, double>(
				out, desc.beginPtr, desc.strideByteOffset, desc.elementCount);
			return;
	}
}


class GltfLoader {
	uri::Uri gltfFilePath;

	tg::Model model;

	std::vector<PodHandle<ImagePod>> imagePods;
	std::vector<PodHandle<SamplerPod>> samplerPods;
	std::vector<PodHandle<MaterialPod>> materialPods;
	// std::vector<PodHandle<ModelPod>> modelPods;

	void LoadGeometryGroup(
		ModelPod* pod, GeometryGroup& geom, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat);
	void LoadMesh(ModelPod* pod, Mesh& mesh, const tinygltf::Mesh& meshData, const glm::mat4& transformMat);

	void LoadMaterial(MaterialPod* pod, size_t index);

	void LoadImages();
	void LoadSamplers();
	void LoadMaterials();

public:
	GltfLoader(const uri::Uri& path);

	// CHECK: currently loads default scene as model, could also load all the other scenes in the future
	BasePodHandle LoadModel();
};

GltfLoader::GltfLoader(const uri::Uri& path)
	: gltfFilePath(path)
{
	tg::TinyGLTF loader;

	std::string err;
	std::string warn;

	const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.c_str());

	CLOG_WARN(!warn.empty(), "Gltf Load warning for {}: {}", path, warn.c_str());
	CLOG_ABORT(!err.empty(), "Gltf Load error for {}: {}", path, err.c_str());

	LoadImages();
	LoadSamplers();
	LoadMaterials();
}

void GltfLoader::LoadGeometryGroup(
	ModelPod* pod, GeometryGroup& geom, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat)
{
	// mode
	// CHECK: handle non triangle case somewhere in code
	geom.mode = gltfaux::GetGeometryMode(primitiveData.mode);

	// material
	const auto materialIndex = primitiveData.material;

	// If material is -1, we need default material.
	if (materialIndex == -1) {
		// Default material will be placed at last slot.
		geom.materialIndex = static_cast<uint32>(pod->materials.size());
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

void GltfLoader::LoadMesh(ModelPod* pod, Mesh& mesh, const tinygltf::Mesh& meshData, const glm::mat4& transformMat)
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

void GltfLoader::LoadMaterial(MaterialPod* pod, size_t index)
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
	pod->alphaMode = gltfaux::GetAlphaMode(data.alphaMode);

	pod->alphaCutoff = static_cast<float>(data.alphaCutoff);
	// doublesided-ness
	pod->doubleSided = data.doubleSided;


	// NEXT: load normal map default
	auto fillMatTexture
		= [&](auto textureInfo, PodHandle<SamplerPod>& sampler, PodHandle<ImagePod>& image, int32& uvIndex) {
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
			  } // else defaults (NEXT: normal map not correct)
		  };

	// samplers
	auto& baseColorTextureInfo = data.pbrMetallicRoughness.baseColorTexture;
	fillMatTexture(baseColorTextureInfo, pod->baseColorSampler, pod->baseColorImage, pod->baseColorTexcoordIndex);

	auto& emissiveTextureInfo = data.emissiveTexture;
	fillMatTexture(emissiveTextureInfo, pod->emissiveSampler, pod->emissiveImage, pod->emissiveTexcoordIndex);

	auto& normalTextureInfo = data.normalTexture;
	fillMatTexture(normalTextureInfo, pod->normalSampler, pod->normalImage, pod->normalTexcoordIndex);

	auto& metallicRougnessTextureInfo = data.pbrMetallicRoughness.metallicRoughnessTexture;
	fillMatTexture(metallicRougnessTextureInfo, pod->metallicRoughnessSampler, pod->metallicRoughnessImage,
		pod->metallicRoughnessTexcoordIndex);

	auto& occlusionTextureInfo = data.occlusionTexture;
	fillMatTexture(occlusionTextureInfo, pod->occlusionSampler, pod->occlusionImage, pod->occlusionTexcoordIndex);
}

void GltfLoader::LoadImages()
{
	// CHECK: embedded images not supported currently
	for (auto& img : model.images) {
		imagePods.push_back(AssetImporterManager::ImportRequest<ImagePod>(img.uri));
	}
}

void GltfLoader::LoadSamplers()
{
	int32 samplerIndex = 0;
	for (auto& sampler : model.samplers) {

		nlohmann::json data;
		data["sampler"] = samplerIndex++;
		auto samplerPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = sampler.name.empty() ? "Sampler." + std::to_string(samplerIndex) : sampler.name;

		auto& [handle, pod] = AssetImporterManager::CreateEntry<SamplerPod>(samplerPath, name);

		// NEXT:
		pod->minFilter = gltfaux::GetTextureFiltering(sampler.minFilter);
		pod->magFilter = gltfaux::GetTextureFiltering(sampler.magFilter);
		pod->wrapU = gltfaux::GetTextureWrapping(sampler.wrapS);
		pod->wrapV = gltfaux::GetTextureWrapping(sampler.wrapT);
		pod->wrapW = gltfaux::GetTextureWrapping(sampler.wrapR);

		samplerPods.push_back(handle);
	}
}

void GltfLoader::LoadMaterials()
{
	int32 matIndex = 0;
	for (auto& mat : model.materials) {

		nlohmann::json data;
		data["material"] = matIndex++;
		auto matPath = uri::MakeChildJson(gltfFilePath, data);

		std::string name = mat.name.empty() ? "Mat." + std::to_string(matIndex) : mat.name;

		auto& [handle, pod] = AssetImporterManager::CreateEntry<MaterialPod>(matPath, name);

		LoadMaterial(pod, matIndex);

		materialPods.push_back(handle);
	}
}

BasePodHandle GltfLoader::LoadModel()
{
	auto& [handle, pod] // TODO:
		= AssetImporterManager::CreateEntry<ModelPod>(gltfFilePath, std::string(uri::GetFilenameNoExt(gltfFilePath)));

	int32 scene = model.defaultScene >= 0 ? model.defaultScene : 0;

	auto& defaultScene = model.scenes.at(scene);

	std::function<bool(const std::vector<int>&, glm::mat4)> RecurseChildren;
	RecurseChildren = [&](const std::vector<int>& childrenIndices, glm::mat4 parentTransformMat) {
		for (auto& nodeIndex : childrenIndices) {
			auto& childNode = model.nodes.at(nodeIndex);

			glm::mat4 localTransformMat = glm::mat4(1.f);

			// When matrix is defined, it must be decomposable to TRS.
			if (!childNode.matrix.empty()) {
				for (int32 row = 0; row < 4; ++row) {
					for (int32 column = 0; column < 4; ++column) {
						localTransformMat[row][column] = static_cast<float>(childNode.matrix[column + 4 * row]);
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

	// NEXT: default material (should we change the way materials are stored in models?)
	pod->materials.push_back({});
}

} // namespace

BasePodHandle GltfImporter::Import(const fs::path& path)
{
	// const auto finalPath = path.generic_string();

	// auto& [handle, pod] = AssetImporterManager::CreateEntry<ModelPod>(path.generic_string(),
	// path.filename().string());


	GltfLoader loader{ path.generic_string() };
	return loader.LoadModel();
}
