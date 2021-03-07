#include "GltfSceneToStaticMeshLoader.h"

#include "assets/AssetImporterManager.h"
#include "assets/importers/gltf/GltfCache.h"
#include "assets/importers/gltf/GltfUtl.h"
#include "assets/pods/Mesh.h"


namespace gltfutl {
GltfSceneToStaticMeshLoader::GltfSceneToStaticMeshLoader(GltfCache& cache, tg::Scene& scene)
	: m_cache(cache)
{
	auto&& [handle, pod] = AssetImporterManager->CreateEntry<Mesh>(
		m_cache.gltfFilePath, std::string(uri::GetFilenameNoExt(m_cache.gltfFilePath)));

	m_loadedPod = handle;

	std::function<bool(const std::vector<int>&, glm::mat4)> RecurseChildren;
	RecurseChildren = [&](const std::vector<int>& childrenIndices, glm::mat4 parentTransformMat) {
		for (auto& nodeIndex : childrenIndices) {
			auto& childNode = m_cache.gltfData->nodes.at(nodeIndex);

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
				auto& gltfMesh = m_cache.gltfData->meshes.at(childNode.mesh);

				for (auto& prim : gltfMesh.primitives) {

					if (prim.mode != TINYGLTF_MODE_TRIANGLES) {
						LOG_WARN("Unsupported primitive data mode, filename: {}, mesh name: {}", m_cache.filename,
							gltfMesh.name);
						continue;
					}


					// material
					// If material is -1, we use default material.
					int32 materialIndex = prim.material != -1 ? prim.material //
															  : static_cast<int32>(cache.materialPods.size() - 1);

					CLOG_ABORT(materialIndex >= pod->geometrySlots.size(),
						"Material index higher than slot count. Gltf file contains a geometry group with material "
						"index higher than "
						"the total materials included.");

					GeometrySlot& slot = pod->geometrySlots[materialIndex];
					auto&& [lastBegin, lastSize]
						= LoadBasicDataIntoGeometrySlot<GeometrySlot, Vertex>(slot, *m_cache.gltfData, prim);

					// Bake transform
					const auto invTransMat = glm::transpose(glm::inverse(glm::mat3(localTransformMat)));
					for (size_t i = lastBegin; i < lastSize; ++i) {

						slot.vertices[i].position = localTransformMat * glm::vec4(slot.vertices[i].position, 1.f);

						slot.vertices[i].normal = glm::normalize(invTransMat * slot.vertices[i].normal);
						slot.vertices[i].tangent = glm::normalize(invTransMat * slot.vertices[i].tangent);
					}
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
