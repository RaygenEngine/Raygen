#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/ModelPod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/loaders/DummyLoader.h"
#include "asset/util/GltfAux.h"

#include "tinygltf/tiny_gltf.h"

namespace GltfModelLoader {
// TODO: Refactor: implementing this as a local struct with member functions can reduce the function arguments.
namespace {
	namespace tg = tinygltf;

	// TODO: This code has high maintenance cost due to its complexity.
	// the complexity here is mostly used to protect against 'incompatible' conversion types.
	// The maintenance cost may not be worth the 'safe' conversions.

	//// Copy with strides from a C array-like to a vector while performing normal type conversion.
	// template<typename ComponentType, typename OutputType>
	// void CopyToVector(std::vector<OutputType>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount,
	//	size_t componentCount)
	//{
	//	for (int32 i = 0; i < elementCount; ++i) {
	//		byte* elementPtr = &beginPtr[perElementOffset * i];
	//		ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

	//		for (uint32 c = 0; c < componentCount; ++c) {
	//			if constexpr (std::is_same_v<double, ComponentType>) {
	//				result[i][c] = static_cast<float>(data[c]); // explicitly convert any double input to float.
	//			}
	//			else {
	//				result[i][c]
	//					= data[c]; // normal type conversion, should be able to convert most of the required stuff
	//			}
	//		}
	//	}
	//}

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

		BufferComponentType componentType; // this particular model's underlying buffer type to read as.

		{
			size_t beginByteOffset;
			const tinygltf::Accessor& accessor = modelData.accessors.at(accessorIndex);
			const tinygltf::BufferView& bufferView = modelData.bufferViews.at(accessor.bufferView);
			const tinygltf::Buffer& gltfBuffer = modelData.buffers.at(bufferView.buffer);


			componentType = GltfAux::GetComponentType(accessor.componentType);
			elementCount = accessor.count;
			beginByteOffset = accessor.byteOffset + bufferView.byteOffset;
			strideByteOffset = accessor.ByteStride(bufferView);
			componentCount = utl::GetElementComponentCount(GltfAux::GetElementType(accessor.type));
			beginPtr = const_cast<byte*>(&gltfBuffer.data[beginByteOffset]);
		}
		CLOG_ASSERT(componentCount != 1, "Found indicies of 2 components in gltf file.");
		out.resize(elementCount);

		// This will generate EVERY possible mapping of Output -> ComponentType conversion
		// fix this later and provide empty specializations of "incompatible" types (eg: float -> int)
		// TODO: This code will produce warnings for every type conversion that is considered 'unsafe'

		switch (componentType) {
				// Conversions from signed to unsigned types are "implementation defined".
				// This code assumes the implementation will not do any bit arethmitic from signed x to unsigned x.

			case BufferComponentType::BYTE:
			case BufferComponentType::UNSIGNED_BYTE: {
				CopyToVector<unsigned char>(out, beginPtr, strideByteOffset, elementCount);
				return;
			}
			case BufferComponentType::SHORT: {
				CopyToVector<short>(out, beginPtr, strideByteOffset, elementCount);
				return;
			}
			case BufferComponentType::UNSIGNED_SHORT: {
				CopyToVector<unsigned short>(out, beginPtr, strideByteOffset, elementCount);
				return;
			}
			case BufferComponentType::INT: {
				CopyToVector<int>(out, beginPtr, strideByteOffset, elementCount);
				return;
			}
			case BufferComponentType::UNSIGNED_INT: {
				CopyToVector<uint32>(out, beginPtr, strideByteOffset, elementCount);
				return;
			}
			case BufferComponentType::FLOAT:
			case BufferComponentType::DOUBLE:
			case BufferComponentType::INVALID: return;
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
			else if constexpr (std::is_same_v<float, ComponentType>) { // NOLINT
				result[i].position[0] = data[0];
				result[i].position[1] = data[1];
				result[i].position[2] = data[2];
			}
			else {
				assert(false);
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
			else if constexpr (std::is_same_v<float, ComponentType>) { // NOLINT
				result[i].normal[0] = data[0];
				result[i].normal[1] = data[1];
				result[i].normal[2] = data[2];
			}
			else {
				assert(false);
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

			if constexpr (std::is_same_v<double, ComponentType>) { // NOLINT
				result[i].tangent[0] = static_cast<float>(data[0]);
				result[i].tangent[1] = static_cast<float>(data[1]);
				result[i].tangent[2] = static_cast<float>(data[2]);
			}
			else if constexpr (std::is_same_v<float, ComponentType>) { // NOLINT
				result[i].tangent[0] = data[0];
				result[i].tangent[1] = data[1];
				result[i].tangent[2] = data[2];
			}
			else {
				assert(false);
			}
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
			else if constexpr (std::is_same_v<float, ComponentType>) { // NOLINT
				result[i].textCoord0[0] = data[0];
				result[i].textCoord0[1] = data[1];
			}
			else {
				assert(false);
			}
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
			else if constexpr (std::is_same_v<float, ComponentType>) { // NOLINT
				result[i].textCoord1[0] = data[0];
				result[i].textCoord1[1] = data[1];
			}
			else {
				assert(false);
			}
		}
	}

	template<size_t VertexElementIndex>
	void LoadIntoVertextData(const tg::Model& modelData, int32 accessorIndex, std::vector<VertexData>& out)
	{
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

		BufferComponentType componentType; // this particular model's underlying buffer type to read as.

		{
			size_t beginByteOffset;
			const tinygltf::Accessor& accessor = modelData.accessors.at(accessorIndex);
			const tinygltf::BufferView& bufferView = modelData.bufferViews.at(accessor.bufferView);
			const tinygltf::Buffer& gltfBuffer = modelData.buffers.at(bufferView.buffer);


			componentType = GltfAux::GetComponentType(accessor.componentType);
			elementCount = accessor.count;
			beginByteOffset = accessor.byteOffset + bufferView.byteOffset;
			strideByteOffset = accessor.ByteStride(bufferView);
			componentCount = utl::GetElementComponentCount(GltfAux::GetElementType(accessor.type));
			beginPtr = const_cast<byte*>(&gltfBuffer.data[beginByteOffset]);
		}

		switch (componentType) {
				// Conversions from signed to unsigned types are "implementation defined".
				// This code assumes the implementation will not do any bit arethmitic from signed x to unsigned x.

			case BufferComponentType::BYTE:
			case BufferComponentType::UNSIGNED_BYTE:
			case BufferComponentType::SHORT:
			case BufferComponentType::UNSIGNED_SHORT:
			case BufferComponentType::INT:
			case BufferComponentType::UNSIGNED_INT: assert(false);
			case BufferComponentType::FLOAT:
				LoadIntoVertextData_Selector<VertexElementIndex, float>(out, beginPtr, strideByteOffset, elementCount);
				return;
			case BufferComponentType::DOUBLE:
				LoadIntoVertextData_Selector<VertexElementIndex, double>(out, beginPtr, strideByteOffset, elementCount);
				return;
			case BufferComponentType::INVALID: return;
		}
	}

	template<size_t VertexElementIndex, typename ComponentType>
	void LoadIntoVertextData_Selector(
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


	bool LoadGeometryGroup(ModelPod* pod, GeometryGroup& geom, const tinygltf::Model& modelData,
		const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat, bool& requiresDefaultMaterial)
	{
		// mode
		geom.mode = GltfAux::GetGeometryMode(primitiveData.mode);

		// material
		const auto materialIndex = primitiveData.material;

		// If material is -1, we need default material.
		if (materialIndex == -1) {
			requiresDefaultMaterial = true;
			// Default material will be placed at last slot.
			geom.materialIndex = static_cast<uint32>(pod->materials.size());
		}
		else {
			geom.materialIndex = materialIndex;
		}

		auto it = std::find_if(begin(primitiveData.attributes), end(primitiveData.attributes),
			[](auto& pair) { return utl::CaseInsensitiveCompare(pair.first, "POSITION"); });


		size_t vertexCount = modelData.accessors.at(it->second).count;
		geom.vertices.resize(vertexCount);

		// indexing
		const auto indicesIndex = primitiveData.indices;

		if (indicesIndex != -1) {
			ExtractIndicesInto(modelData, indicesIndex, geom.indices);
		}
		else {
			geom.indices.resize(vertexCount);
			for (int32 i = 0; i < vertexCount; ++i) {
				geom.indices[i] = i;
			}
		}


		bool missingNormals = true;
		bool missingTangents = true;
		bool missingTexcoord0 = true;
		bool missingTexcoord1 = true;

		// attributes
		for (auto& attribute : primitiveData.attributes) {
			const auto& attrName = attribute.first;
			int32 index = attribute.second;

			if (utl::CaseInsensitiveCompare(attrName, "POSITION")) {
				LoadIntoVertextData<0>(modelData, index, geom.vertices);
			}
			else if (utl::CaseInsensitiveCompare(attrName, "NORMAL")) {
				LoadIntoVertextData<1>(modelData, index, geom.vertices);
				missingNormals = false;
			}
			else if (utl::CaseInsensitiveCompare(attrName, "TANGENT")) {
				LoadIntoVertextData<2>(modelData, index, geom.vertices);
				missingTangents = false;
			}
			else if (utl::CaseInsensitiveCompare(attrName, "TEXCOORD_0")) {
				LoadIntoVertextData<3>(modelData, index, geom.vertices);
				missingTexcoord0 = false;
			}
			else if (utl::CaseInsensitiveCompare(attrName, "TEXCOORD_1")) {
				LoadIntoVertextData<4>(modelData, index, geom.vertices);
				missingTexcoord1 = false;
			}
		}


		for (auto& v : geom.vertices) {
			v.position = transformMat * glm::vec4(v.position, 1.f);
		}

		if (!missingNormals) {
			const auto invTransMat = glm::transpose(glm::inverse(glm::mat3(transformMat)));
			for (auto& v : geom.vertices) {
				v.normal = invTransMat * v.normal;
			}
		}
		else {
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

		// TODO test better calculations (using uv layer 0?) also text tangent handedness (urgently)
		// calculate missing tangents (and bitangents)
		if (missingTangents) {
			for (auto& v : geom.vertices) {
				const auto c1 = glm::cross(v.normal, glm::vec3(0.0, 0.0, 1.0));
				const auto c2 = glm::cross(v.normal, glm::vec3(0.0, 1.0, 0.0));

				v.tangent = glm::length2(c1) > glm::length2(c2) ? glm::vec4(glm::normalize(c1), 1.0f)
																: glm::vec4(glm::normalize(c2), -1.f);
			}
		}
		for (auto& v : geom.vertices) {
			// TODO: handness issues bitangent = cross(normal, tangent.xyz) * tangent.w
			v.bitangent = glm::normalize(glm::cross(v.normal, glm::vec3(v.tangent)));
		}

		// calculate other baked data
		return true;
	}

	bool LoadMesh(ModelPod* pod, Mesh& mesh, const tinygltf::Model& modelData, const tinygltf::Mesh& meshData,
		const glm::mat4& transformMat, bool& requiresDefaultMaterial)
	{
		mesh.geometryGroups.resize(meshData.primitives.size());

		// primitives
		for (int32 i = 0; i < mesh.geometryGroups.size(); ++i) {
			const auto geomName = "geom_group" + std::to_string(i);

			auto& primitiveData = meshData.primitives.at(i);

			// if one of the geometry groups fails to load
			if (!LoadGeometryGroup(
					pod, mesh.geometryGroups[i], modelData, primitiveData, transformMat, requiresDefaultMaterial)) {
				LOG_ERROR("Failed to load geometry group, name: {}", geomName);
				return false;
			}
		}
		return true;
	}
} // namespace

inline bool Load(ModelPod* pod, const uri::Uri& path)
{
	const auto pPath = uri::GetDiskPath(path);
	auto pParent = AssetManager::GetOrCreate<GltfFilePod>(pPath + "{}");

	const tinygltf::Model& model = pParent->data;

	int32 scene = model.defaultScene;

	if (scene < 0) {
		scene = 0;
	}

	auto& defaultScene = model.scenes.at(scene);


	int32 matIndex = 0;
	for (auto& gltfMaterial : model.materials) {
		nlohmann::json data;
		data["material"] = matIndex++;
		auto matPath = uri::MakeChildJson(path, data);
		pod->materials.push_back(AssetManager::GetOrCreate<MaterialPod>(matPath));

		if (gltfMaterial.name.empty()) {
			AssetManager::SetPodName(matPath, "Mat." + std::to_string(matIndex));
		}
		else {
			AssetManager::SetPodName(matPath, gltfMaterial.name);
		}
	}
	bool requiresDefaultMaterial = false;

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

				localTransformMat = utl::GetTransformMat(translation, orientation, scale);
			}

			localTransformMat = parentTransformMat * localTransformMat;

			// TODO: check instancing cases
			// load mesh if exists
			if (childNode.mesh != -1) {
				auto& gltfMesh = model.meshes.at(childNode.mesh);

				Mesh mesh;

				// if missing mesh
				if (!LoadMesh(pod, mesh, model, gltfMesh, localTransformMat, requiresDefaultMaterial)) {
					LOG_ERROR("Failed to load mesh, name: {}", gltfMesh.name);
					return false;
				}
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

	bool result = RecurseChildren(defaultScene.nodes, glm::mat4(1.f));

	if (requiresDefaultMaterial) {
		pod->materials.push_back(GET_CUSTOM_POD(MaterialPod, ""));
	}

	return result;
}
}; // namespace GltfModelLoader
