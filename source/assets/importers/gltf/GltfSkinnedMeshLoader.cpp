#include "pch.h"
#include "GltfSkinnedMeshLoader.h"

#include "assets/AssetImporterManager.h"
#include "assets/importers/gltf/GltfCache.h"
#include "assets/importers/gltf/GltfUtl.h"

namespace gltfutl {
GltfSkinnedMeshLoader::GltfSkinnedMeshLoader(GltfCache& cache, uint32 skinIndex, tg::Skin& skin)
	: m_cache(cache)
{
	auto& [handle, pod] = ImporterManager->CreateEntry<SkinnedMesh>( // WIP:
		m_cache.gltfFilePath, std::string(uri::GetFilenameNoExt(m_cache.gltfFilePath)) + "_skinned_" + skin.name);

	m_loadedPod = handle;

	AccessorDescription desc(m_cache.gltfData, skin.inverseBindMatrices);

	ExtractMatrices4Into(m_cache.gltfData, skin.inverseBindMatrices, pod->jointMatrices);

	pod->parentJoint.resize(pod->jointMatrices.size());

	// Create a slot for each material (+ missing material)
	// We will then iterate gltf geometry groups and append to slot[gg.materialIndex] vertex and index data.
	// When finished we will cleanup any slotgroups that have index == 0; Deleting will be fast because we will just
	// move the underlying vertex buffer vectors
	pod->skinnedGeometrySlots.resize(m_cache.materialPods.size());
	pod->materials = m_cache.materialPods;


	// TODO: premultiply transforms that affect skeleton and the rest of the hierarchy

	for (auto node : m_cache.gltfData.nodes) {
		if (node.skin == skinIndex) {
			// load mesh if exists
			if (node.mesh != -1) {
				auto& gltfMesh = m_cache.gltfData.meshes.at(node.mesh);

				for (auto& prim : gltfMesh.primitives) {

					CLOG_ABORT(prim.mode != TINYGLTF_MODE_TRIANGLES, "Unsupported primitive data mode {}", //
						m_cache.filename);

					// material
					// If material is -1, we use default material.
					int32 materialIndex = prim.material != -1 ? prim.material //
															  : static_cast<int32>(cache.materialPods.size() - 1);

					CLOG_ABORT(materialIndex >= pod->skinnedGeometrySlots.size(),
						"Material index higher than slot count. Gltf file contains a geometry group with material "
						"index higher than "
						"the total materials included.");

					SkinnedGeometrySlot& slot = pod->skinnedGeometrySlots[materialIndex];
					LoadGeometrySlotBasicData<SkinnedGeometrySlot, SkinnedVertex>(slot, m_cache, prim);

					// Also load extra stuff
					int32 joints0Index = -1;
					int32 weights0Index = -1;

					// extra attributes
					for (auto& attribute : prim.attributes) {
						const auto& attrName = attribute.first;
						int32 index = attribute.second;

						if (str::equalInsensitive(attrName, "JOINTS_0")) {
							joints0Index = index;
						}
						else if (str::equalInsensitive(attrName, "WEIGHTS_0")) {
							weights0Index = index;
						}
					}

					// JOINTS
					LoadIntoVertexData<SkinnedVertex, 4>(m_cache.gltfData, joints0Index, slot.vertices);

					// WEIGHTS
					LoadIntoVertexData<SkinnedVertex, 5>(m_cache.gltfData, weights0Index, slot.vertices);
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


	// NEXT: skeleton node
	RecurseChildren(skin.joints[0], UINT32_MAX);

	// Cleanup empty geometry slots
	for (int32 i = static_cast<int32>(pod->skinnedGeometrySlots.size()) - 1; i >= 0; i--) {
		if (pod->skinnedGeometrySlots[i].vertices.empty()) {
			pod->skinnedGeometrySlots.erase(pod->skinnedGeometrySlots.begin() + i);
			pod->materials.erase(pod->materials.begin() + i);
		}
	}
}
} // namespace gltfutl
