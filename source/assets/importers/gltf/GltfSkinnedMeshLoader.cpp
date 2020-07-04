#include "pch.h"
#include "GltfSkinnedMeshLoader.h"

#include "assets/AssetImporterManager.h"
#include "assets/importers/gltf/GltfCache.h"

namespace gltfutl {

void GltfSkinnedMeshLoader::LoadGeometryGroup(SkinnedGeometryGroup& geom, const tinygltf::Primitive& primitiveData)
{
	CLOG_WARN(primitiveData.mode != TINYGLTF_MODE_TRIANGLES, "Unsupported primitive data mode {}", m_cache.filename);

	// material
	const auto materialIndex = primitiveData.material;

	// If material is -1, we need default material.
	if (materialIndex == -1) {
		// Default material will be placed at last slot.
		geom.materialIndex = static_cast<uint32>(m_cache.materialPods.size() - 1);
		m_tempModelRequiresDefaultMat = true;
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
	int32 joints0Index = -1;
	int32 weights0Index = -1;

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
		else if (str::equalInsensitive(attrName, "JOINTS_0")) {
			joints0Index = index;
		}
		else if (str::equalInsensitive(attrName, "WEIGHTS_0")) {
			weights0Index = index;
		}
	}

	// load in this order

	// POSITIONS
	if (positionsIndex != -1) {
		LoadIntoVertexData<SkinnedVertex, 0>(m_cache.gltfData, positionsIndex, geom.vertices);
	}
	else {
		LOG_ABORT("Model does not have any positions...");
	}

	// NORMALS
	if (normalsIndex != -1) {
		LoadIntoVertexData<SkinnedVertex, 1>(m_cache.gltfData, normalsIndex, geom.vertices);
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
		LoadIntoVertexData<SkinnedVertex, 3>(m_cache.gltfData, texcoords0Index, geom.vertices);
	}
	else {
		LOG_DEBUG("Model missing first uv map, not handled");
	}

	// TANGENTS, BITANGENTS
	if (tangentsIndex != -1) {
		LoadIntoVertexData<SkinnedVertex, 2>(m_cache.gltfData, tangentsIndex, geom.vertices);
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

	// JOINTS
	LoadIntoVertexData<SkinnedVertex, 4>(m_cache.gltfData, joints0Index, geom.vertices);

	// WEIGHTS
	LoadIntoVertexData<SkinnedVertex, 5>(m_cache.gltfData, weights0Index, geom.vertices);
}

GltfSkinnedMeshLoader::GltfSkinnedMeshLoader(GltfCache& cache, uint32 skinIndex, tg::Skin& skin)
	: m_cache(cache)
{
	auto& [handle, pod] = ImporterManager->CreateEntry<SkinnedMesh>( // WIP:
		m_cache.gltfFilePath, std::string(uri::GetFilenameNoExt(m_cache.gltfFilePath)) + "_skinned_" + skin.name);

	m_loadedPod = handle;

	AccessorDescription desc(m_cache.gltfData, skin.inverseBindMatrices);

	ExtractMatrices4Into(m_cache.gltfData, skin.inverseBindMatrices, pod->jointMatrices);

	pod->parentJoint.resize(pod->jointMatrices.size());

	// TODO: premultiply transforms that affect skeleton and the rest of the hierarchy

	for (auto node : m_cache.gltfData.nodes) {
		if (node.skin == skinIndex) {
			// load mesh if exists
			if (node.mesh != -1) {
				auto& gltfMesh = m_cache.gltfData.meshes.at(node.mesh);

				for (auto& prim : gltfMesh.primitives) {
					pod->geometryGroups.emplace_back();
					LoadGeometryGroup(pod->geometryGroups.back(), prim);
				}
				break;
			}
		}
	}

	std::function<bool(uint32, uint32)> RecurseChildren;
	RecurseChildren = [&](uint32 parentIndex, uint32 parentJointIndex) {
		auto parentNode = m_cache.gltfData.nodes.at(parentIndex);

		if (auto it = std::find(skin.joints.begin(), skin.joints.end(), parentIndex); it != skin.joints.end()) {
			// this is a joint node
			auto dist = std::distance(skin.joints.begin(), it);
			pod->parentJoint[dist] = parentJointIndex;
			parentJointIndex = static_cast<uint32>(dist);
		}


		for (auto& childIndex : parentNode.children) {
			auto& childNode = m_cache.gltfData.nodes.at(childIndex);


			RecurseChildren(childIndex, parentJointIndex);
		}
		return true;
	};

	RecurseChildren(skin.joints[0], UINT32_MAX);

	LOG_REPORT("VS SUCKS");
}
} // namespace gltfutl
