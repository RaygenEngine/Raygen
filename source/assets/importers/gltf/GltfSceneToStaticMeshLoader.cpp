#include "pch.h"
#include "GltfSceneToStaticMeshLoader.h"

#include "assets/importers/gltf/GltfCache.h"
#include "assets/AssetImporterManager.h"
#include "assets/UriLibrary.h"
#include "core/StringUtl.h"

namespace gltfutl {

void GltfSceneToStaticMeshLoader::LoadGeometryGroup(
	GeometryGroup& geom, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat)
{
	CLOG_WARN(primitiveData.mode != TINYGLTF_MODE_TRIANGLES, "Unsupported primitive data mode {}", m_cache.filename);

	// material
	const auto materialIndex = primitiveData.material;

	// If material is -1, we use default material.
	if (materialIndex == -1) {
		// Default material will be placed at last slot.
		geom.materialIndex = static_cast<uint32>(m_cache.materialPods.size() - 1);
	}
	else {
		geom.materialIndex = materialIndex;
	}

	auto it = std::find_if(begin(primitiveData.attributes), end(primitiveData.attributes),
		[](auto& pair) { return str::equalInsensitive(pair.first, "POSITION"); });


	size_t vertexCount = m_cache.gltfData.accessors.at(it->second).count;
	geom.vertices.resize(vertexCount);

	// indexing
	const auto indicesIndex = primitiveData.indices;

	if (indicesIndex != -1) {
		ExtractIndicesInto(m_cache.gltfData, indicesIndex, geom.indices);
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
		LoadIntoVertexData<Vertex, 0>(m_cache.gltfData, positionsIndex, geom.vertices);
	}
	else {
		LOG_ABORT("Model does not have any positions...");
	}

	// NORMALS
	if (normalsIndex != -1) {
		LoadIntoVertexData<Vertex, 1>(m_cache.gltfData, normalsIndex, geom.vertices);
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
		LoadIntoVertexData<Vertex, 3>(m_cache.gltfData, texcoords0Index, geom.vertices);
	}
	else {
		LOG_DEBUG("Model missing first uv map, not handled");
	}

	// TANGENTS, BITANGENTS
	if (tangentsIndex != -1) {
		LoadIntoVertexData<Vertex, 2>(m_cache.gltfData, tangentsIndex, geom.vertices);
	}
	else {
		if (texcoords0Index != -1) {
			LOG_DEBUG("Model missing tangents, calculating using available uv map");

			for (int32 i = 0; i < geom.indices.size(); i += 3) {
				// triangle
				auto p0 = geom.vertices[geom.indices[i]].position;
				auto p1 = geom.vertices[geom.indices[i + 1]].position;
				auto p2 = geom.vertices[geom.indices[i + 2]].position;

				auto uv0 = geom.vertices[geom.indices[i]].uv;
				auto uv1 = geom.vertices[geom.indices[i + 1]].uv;
				auto uv2 = geom.vertices[geom.indices[i + 2]].uv;

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

		v.normal = glm::normalize(invTransMat * v.normal);
		v.tangent = glm::normalize(invTransMat * v.tangent);
		v.bitangent = glm::normalize(invTransMat * v.bitangent);
	}
}

void GltfSceneToStaticMeshLoader::AppendGeometryGroupToSlot(std::vector<GeometrySlot>& slots, GeometryGroup& group)
{
	CLOG_ABORT(group.materialIndex >= slots.size(),
		"Material index higher than slot count. Gltf file contains a geometry group with material index higher than "
		"the total materials included.");

	GeometrySlot& slot = slots[group.materialIndex];

	unsigned int size = slot.vertices.size();
	for (auto& ind : group.indices) {
		ind += size;
	}

	slot.vertices.reserve(slot.vertices.size() + group.vertices.size());
	slot.vertices.insert(slot.vertices.end(), group.vertices.begin(), group.vertices.end());
	slot.indices.insert(slot.indices.end(), group.indices.begin(), group.indices.end());
}

GltfSceneToStaticMeshLoader::GltfSceneToStaticMeshLoader(GltfCache& cache, tg::Scene& scene)
	: m_cache(cache)
{
	auto& [handle, pod] = ImporterManager->CreateEntry<Mesh>(
		m_cache.gltfFilePath, std::string(uri::GetFilenameNoExt(m_cache.gltfFilePath)));

	m_loadedPod = handle;

	std::function<bool(const std::vector<int>&, glm::mat4)> RecurseChildren;
	RecurseChildren = [&](const std::vector<int>& childrenIndices, glm::mat4 parentTransformMat) {
		for (auto& nodeIndex : childrenIndices) {
			auto& childNode = m_cache.gltfData.nodes.at(nodeIndex);

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
				auto& gltfMesh = m_cache.gltfData.meshes.at(childNode.mesh);

				for (auto& prim : gltfMesh.primitives) {
					GeometryGroup gg;
					LoadGeometryGroup(gg, prim, localTransformMat);
					// PERF: Bake this into the loading code, it will save tons of copies
					AppendGeometryGroupToSlot(pod->geometrySlots, gg);
				}
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

	// Create a slot for each material (+ missing material)
	// We will then iterate gltf geometry groups and append to slot[gg.materialIndex] vertex and index data.
	// When finished we will cleanup any slotgroups that have index == 0; Deleting will be fast because we will just
	// move the underlying vertex buffer vectors
	pod->geometrySlots.resize(m_cache.materialPods.size());
	pod->materials = m_cache.materialPods;

	RecurseChildren(scene.nodes, glm::mat4(1.f));

	// Cleanup empty geometry slots
	for (int32 i = static_cast<int32>(pod->geometrySlots.size()) - 1; i >= 0; i--) {
		if (pod->geometrySlots[i].vertices.empty()) {
			pod->geometrySlots.erase(pod->geometrySlots.begin() + i);
			pod->materials.erase(pod->materials.begin() + i);
		}
	}
}

} // namespace gltfutl
