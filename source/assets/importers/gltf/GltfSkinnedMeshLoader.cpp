#include "pch.h"
#include "GltfSkinnedMeshLoader.h"

#include "assets/AssetImporterManager.h"
#include "assets/importers/gltf/GltfCache.h"
#include "assets/importers/gltf/GltfUtl.h"
namespace {

glm::mat4 GetLocalTransformFromGltfNode(tinygltf::Node& node)
{
	glm::mat4 localTransformMat = glm::mat4(1.f);

	// When matrix is defined, it must be decomposable to TRS.
	if (!node.matrix.empty()) {
		for (int32 row = 0; row < 4; ++row) {
			for (int32 column = 0; column < 4; ++column) {
				localTransformMat[row][column] = static_cast<float>(node.matrix[column + 4llu * row]);
			}
		}
	}
	else {
		glm::vec3 translation = glm::vec3(0.f);
		glm::quat orientation = { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 scale = glm::vec3(1.f);

		if (!node.translation.empty()) {
			translation[0] = static_cast<float>(node.translation[0]);
			translation[1] = static_cast<float>(node.translation[1]);
			translation[2] = static_cast<float>(node.translation[2]);
		}

		if (!node.rotation.empty()) {
			orientation[0] = static_cast<float>(node.rotation[0]);
			orientation[1] = static_cast<float>(node.rotation[1]);
			orientation[2] = static_cast<float>(node.rotation[2]);
			orientation[3] = static_cast<float>(node.rotation[3]);
		}

		if (!node.scale.empty()) {
			scale[0] = static_cast<float>(node.scale[0]);
			scale[1] = static_cast<float>(node.scale[1]);
			scale[2] = static_cast<float>(node.scale[2]);
		}

		localTransformMat = math::transformMat(scale, orientation, translation);
	}

	return localTransformMat;
}


} // namespace

namespace gltfutl {


GltfSkinnedMeshLoader::GltfSkinnedMeshLoader(GltfCache& cache, uint32 skinIndex, tg::Skin& skin)
	: m_cache(cache)
{
	auto& [handle, pod] = ImporterManager->CreateEntry<SkinnedMesh>( // WIP:
		m_cache.gltfFilePath, std::string(uri::GetFilenameNoExt(m_cache.gltfFilePath)) + "_skinned_" + skin.name);

	m_loadedPod = handle;

	AccessorDescription desc(m_cache.gltfData, skin.inverseBindMatrices);

	std::vector<glm::mat4> invBindMatrix;

	ExtractMatrices4Into(m_cache.gltfData, skin.inverseBindMatrices, invBindMatrix);


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
					auto& [lastBegin, lastSize]
						= LoadBasicDataIntoGeometrySlot<SkinnedGeometrySlot, SkinnedVertex>(slot, m_cache, prim);

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
					LoadIntoVertexData<SkinnedVertex, 4>(
						m_cache.gltfData, joints0Index, slot.vertices.data() + lastBegin);

					// WEIGHTS
					LoadIntoVertexData<SkinnedVertex, 5>(
						m_cache.gltfData, weights0Index, slot.vertices.data() + lastBegin);
				}
				break;
			}
		}
	}

	std::function<bool(uint32, uint32)> RecurseChildren;
	RecurseChildren = [&](uint32 nodeIndex, uint32 parentJointIndex) {
		auto& node = m_cache.gltfData.nodes.at(nodeIndex);

		if (auto it = std::find(skin.joints.begin(), skin.joints.end(), nodeIndex); it != skin.joints.end()) {
			// this is a joint node
			auto dist = std::distance(skin.joints.begin(), it);
			auto& joint = pod->joints[dist];
			joint.parentJoint = parentJointIndex;
			joint.index = dist;
			joint.inverseBindMatrix = invBindMatrix[dist];
			joint.localTransform = GetLocalTransformFromGltfNode(node);
			joint.name = node.name;

			parentJointIndex = static_cast<uint32>(dist);
		}
		else {
			LOG_ERROR("Gltf Importer: Found non joint in hierarchy.");
		}


		for (auto& childIndex : node.children) {
			auto& childNode = m_cache.gltfData.nodes.at(childIndex);
			RecurseChildren(childIndex, parentJointIndex);
		}
		return true;
	};


	CLOG_WARN(skin.skeleton != skin.joints[0], "Gltf Importer: skin.skeleton = {}, Skin.joints[0] = {}", skin.skeleton,
		skin.joints[0]);

	int32 skeletonRoot = skin.skeleton == -1 ? skin.joints[0] : skin.skeleton;
	pod->joints.resize(skin.joints.size());

	RecurseChildren(skeletonRoot, UINT32_MAX);


	// Cleanup empty geometry slots
	for (int32 i = static_cast<int32>(pod->skinnedGeometrySlots.size()) - 1; i >= 0; i--) {
		if (pod->skinnedGeometrySlots[i].vertices.empty()) {
			pod->skinnedGeometrySlots.erase(pod->skinnedGeometrySlots.begin() + i);
			pod->materials.erase(pod->materials.begin() + i);
		}
	}
}
} // namespace gltfutl
