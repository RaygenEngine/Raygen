#include "pch.h"
#include "GltfSkinnedMeshLoader.h"

#include "assets/AssetImporterManager.h"
#include "assets/importers/gltf/GltfCache.h"
#include "assets/importers/gltf/GltfUtl.h"


#include <glm/gtx/matrix_decompose.hpp>
#include <nlohmann/json.hpp>
#include <queue>


namespace {

void ExtractJointTRSFromNode(tinygltf::Node& node, SkinnedMesh::Joint& joint)
{
	// When matrix is defined, it must be decomposable to TRS.
	if (!node.matrix.empty()) {
		glm::mat4 localTransformMat = glm::mat4(1.f);
		for (int32 row = 0; row < 4; ++row) {
			for (int32 column = 0; column < 4; ++column) {
				localTransformMat[row][column] = static_cast<float>(node.matrix[column + 4llu * row]);
			}
		}

		glm::vec3 skew;
		glm::vec4 persp;

		glm::decompose(localTransformMat, joint.scale, joint.rotation, joint.translation, skew, persp);
		return;
	}


	if (!node.translation.empty()) {
		joint.translation[0] = static_cast<float>(node.translation[0]);
		joint.translation[1] = static_cast<float>(node.translation[1]);
		joint.translation[2] = static_cast<float>(node.translation[2]);
	}
	else {
		joint.translation = glm::vec3(0.f);
	}


	if (!node.rotation.empty()) {
		joint.rotation[0] = static_cast<float>(node.rotation[0]);
		joint.rotation[1] = static_cast<float>(node.rotation[1]);
		joint.rotation[2] = static_cast<float>(node.rotation[2]);
		joint.rotation[3] = static_cast<float>(node.rotation[3]);
	}
	else {
		joint.rotation = { 1.f, 0.f, 0.f, 0.f };
	}

	if (!node.scale.empty()) {
		joint.scale[0] = static_cast<float>(node.scale[0]);
		joint.scale[1] = static_cast<float>(node.scale[1]);
		joint.scale[2] = static_cast<float>(node.scale[2]);
	}
	else {
		joint.scale = glm::vec3(1.f);
	}
}

} // namespace

namespace gltfutl {

void GltfSkinnedMeshLoader::LoadSkinMesh()
{
	// Create a slot for each material (+ defualt material)
	// We will then iterate gltf geometry groups and append to slot[gg.materialIndex] vertex and index data.
	// When finished we will cleanup any slotgroups that have index == 0; Deleting will be fast because we will just
	// move the underlying vertex buffer vectors

	// Default material is located at the last index of cache.materialPods (will get removed at a later stage if no
	// vertices found)
	skinPod->skinnedGeometrySlots.resize(cache.materialPods.size());
	skinPod->materials = cache.materialPods;

	for (auto node : cache.gltfData.nodes) {
		if (node.skin == skinIndex) {
			// load mesh if exists
			if (node.mesh != -1) {
				auto& gltfMesh = cache.gltfData.meshes.at(node.mesh);

				for (auto& prim : gltfMesh.primitives) {

					CLOG_ABORT(prim.mode != TINYGLTF_MODE_TRIANGLES, "Unsupported primitive data mode {}", //
						cache.filename);

					// material
					// If material is -1, we use default material.
					int32 materialIndex = prim.material != -1 ? prim.material //
															  : static_cast<int32>(cache.materialPods.size() - 1);

					CLOG_ABORT(materialIndex >= skinPod->skinnedGeometrySlots.size(),
						"Material index higher than slot count. Gltf file contains a geometry group with material "
						"index higher than "
						"the total materials included.");

					SkinnedGeometrySlot& slot = skinPod->skinnedGeometrySlots[materialIndex];
					auto& [lastBegin, lastSize]
						= LoadBasicDataIntoGeometrySlot<SkinnedGeometrySlot, SkinnedVertex>(slot, cache, prim);

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
						cache.gltfData, joints0Index, slot.vertices.data() + lastBegin);

					// WEIGHTS
					LoadIntoVertexData<SkinnedVertex, 5>(
						cache.gltfData, weights0Index, slot.vertices.data() + lastBegin);
				}
				break;
			}
		}
	}

	// Cleanup empty geometry slots
	for (int32 i = static_cast<int32>(skinPod->skinnedGeometrySlots.size()) - 1; i >= 0; i--) {
		if (skinPod->skinnedGeometrySlots[i].vertices.empty()) {
			skinPod->skinnedGeometrySlots.erase(skinPod->skinnedGeometrySlots.begin() + i);
			skinPod->materials.erase(skinPod->materials.begin() + i);
		}
	}
}

void GltfSkinnedMeshLoader::SortJoints()
{
	size_t jointCount = skinPod->joints.size();

	// Store the children for each node.
	std::vector<std::vector<int32>> children(jointCount);
	for (auto& j : skinPod->joints) {
		if (!j.IsRoot()) {
			children[j.parentJoint].push_back(j.index);
		}
	}

	// Remap[i] contains the index in the original joints array that should be moved to i position
	std::vector<int32> jointRemapInverse;
	std::vector<SkinnedMesh::Joint> newJoints;


	// Tree bfs (PERF: dfs may increase animator performance due to caching ?)
	std::queue<int32> placeQueue;
	placeQueue.push(0);
	while (!placeQueue.empty()) {
		int32 current = placeQueue.front();
		placeQueue.pop();

		for (auto child : children[current]) {
			placeQueue.push(child);
		}

		jointRemapInverse.push_back(current);

		if (!skinPod->joints[current].IsRoot()) {
			auto it = std::find_if(newJoints.begin(), newJoints.end(),
				[&](auto& joint) { return joint.index == skinPod->joints[current].parentJoint; });

			CLOG_ABORT(it == newJoints.end(), "Joint not found, programmer error in sorting algorithm");
			skinPod->joints[current].parentJoint = std::distance(newJoints.begin(), it);
		}

		newJoints.emplace_back(std::move(skinPod->joints[current]));
	}

	skinPod->joints.swap(newJoints);

	// "Flip" the remapping so that remap[oldJointIndex] = newJointIndex
	jointRemap.resize(jointCount);

	for (int32 i = 0; i < jointCount; ++i) {
		jointRemap[jointRemapInverse[i]] = i;
		skinPod->joints[i].index = i;
	}


	// Adjust all vertex weights to new joints
	for (auto& slot : skinPod->skinnedGeometrySlots) {
		for (auto& vert : slot.vertices) {
			vert.joint.x = jointRemap[vert.joint.x];
			vert.joint.y = jointRemap[vert.joint.y];
			vert.joint.z = jointRemap[vert.joint.z];
			vert.joint.w = jointRemap[vert.joint.w];
		}
	}
}

int32 GltfSkinnedMeshLoader::NodeToJoint(int32 nodeIndex)
{
	if (auto it = std::find(gltfSkin.joints.begin(), gltfSkin.joints.end(), nodeIndex); it != gltfSkin.joints.end()) {
		int32 jointIndex = std::distance(gltfSkin.joints.begin(), it);
		if (jointRemap.empty()) {
			return jointIndex;
		}
		return jointRemap[jointIndex];
	}
	return -1; // Not a joint
}

GltfSkinnedMeshLoader::GltfSkinnedMeshLoader(GltfCache& inCache, uint32 inSkinIndex, tg::Skin& skin)
	: cache(inCache)
	, gltfSkin(skin)
	, skinIndex(inSkinIndex)
{
	auto& [handle, pod] = AssetImporterManager->CreateEntry<SkinnedMesh>(
		cache.gltfFilePath, std::string(uri::GetFilenameNoExt(cache.gltfFilePath)) + "_skinned_" + skin.name);

	skinHandle = handle;
	skinPod = pod;


	AccessorDescription desc(cache.gltfData, skin.inverseBindMatrices);

	std::vector<glm::mat4> invBindMatrix;

	ExtractMatrices4Into(cache.gltfData, skin.inverseBindMatrices, invBindMatrix);


	size_t jointCount = invBindMatrix.size();

	LoadSkinMesh();

	bool jointsNeedSorting = false;

	std::function<bool(int32, int32)> RecurseChildren;
	RecurseChildren = [&](int32 nodeIndex, int32 parentJointIndex) {
		auto& node = cache.gltfData.nodes.at(nodeIndex);
		int32 jointIndex = NodeToJoint(nodeIndex);

		if (jointIndex >= 0) {
			// this is a joint node

			auto& joint = pod->joints[jointIndex];
			joint.parentJoint = parentJointIndex;
			joint.index = jointIndex;
			if (joint.index < joint.parentJoint && joint.index > 0) {
				jointsNeedSorting = true;
			}

			joint.inverseBindMatrix = invBindMatrix[jointIndex];
			ExtractJointTRSFromNode(node, joint);
			joint.name = node.name;

			parentJointIndex = jointIndex;
		}
		else {
			LOG_ERROR("Gltf Importer: Found non joint in hierarchy.");
		}


		for (auto& childIndex : node.children) {
			auto& childNode = cache.gltfData.nodes.at(childIndex);
			RecurseChildren(childIndex, parentJointIndex);
		}
		return true;
	};


	CLOG_WARN(skin.skeleton != skin.joints[0], "Gltf Importer: skin.skeleton = {}, Skin.joints[0] = {}", skin.skeleton,
		skin.joints[0]);

	int32 skeletonRoot = skin.skeleton == -1 ? skin.joints[0] : skin.skeleton;
	pod->joints.resize(jointCount);

	RecurseChildren(skeletonRoot, SkinnedMesh::c_rootParentJointIndex);
	CLOG_WARN(jointsNeedSorting, "Gltf Importer: Joints need sorting for: {}", cache.filename);

	if (jointsNeedSorting) {
		SortJoints();
	}

	LoadAnimations();
}


void GltfSkinnedMeshLoader::LoadAnimations()
{
	for (int32 animationIndex = 0; auto& anim : cache.gltfData.animations) {

		nlohmann::json data;
		data["animation"] = animationIndex;
		auto animPath = uri::MakeChildJson(cache.gltfFilePath, data);

		std::string name = anim.name.empty() ? cache.filename + "_Anim_" + std::to_string(animationIndex) : anim.name;
		auto& [handle, pod] = AssetImporterManager->CreateEntry<Animation>(animPath, name);


		float maxInputTime = 0.f;

		// load samplers
		for (auto& animSampler : anim.samplers) {
			AnimationSampler as{};

			as.interpolation = GetInterpolationMethod(animSampler.interpolation);

			// inputs
			AccessorDescription desc0(cache.gltfData, animSampler.input);
			as.inputs.resize(desc0.elementCount);
			CopyToFloatVector(as.inputs, desc0.beginPtr, desc0.strideByteOffset, desc0.elementCount);

			maxInputTime = std::max(maxInputTime, as.inputs.back());

			// outputs
			AccessorDescription desc1(cache.gltfData, animSampler.output);


			CLOG_ABORT(desc1.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT,
				"Normalized integer animation outputs are not yet handled. See GltfUtl TODO function for conversion "
				"and implement in the future");

			size_t byteChunkSize = tinygltf::GetComponentSizeInBytes(desc1.componentType) * desc1.componentCount;
			as.outputs.resize(desc1.elementCount);

			for (uint32 i = 0; i < desc1.elementCount; ++i) {
				const void* elementPtr = &desc1.beginPtr[desc1.strideByteOffset * i];
				switch (desc1.accessorType) {
					case TINYGLTF_TYPE_VEC3: {
						const glm::vec3* elem = static_cast<const glm::vec3*>(elementPtr);
						as.outputs[i] = glm::vec4(*elem, 0.0f);
						break;
					}
					case TINYGLTF_TYPE_VEC4: {
						const glm::vec4* elem = static_cast<const glm::vec4*>(elementPtr);
						as.outputs[i] = *elem;
						break;
					}
					default: LOG_ERROR("Incorrect tinygltf type found in AnimationOutput");
				}
			}

			pod->samplers.emplace_back(as);
		}

		// load channels
		for (auto& animCh : anim.channels) {
			AnimationChannel ch{};

			ch.path = GetAnimationPath(animCh.target_path);
			ch.samplerIndex = animCh.sampler;
			ch.targetJoint = NodeToJoint(animCh.target_node);

			CLOG_ERROR(cache.gltfData.nodes[animCh.target_node].name != cache.gltfData.nodes[animCh.target_node].name,
				"Gltf Importer: missmatched remap joint to node names: node: {} joint: {}",
				cache.gltfData.nodes[animCh.target_node].name, skinPod->joints[ch.targetJoint].name);

			CLOG_ERROR(ch.targetJoint == -1, "Gltf Importer: Found channel that moves a non joint.");
			pod->channels.emplace_back(ch);
		}

		pod->time = maxInputTime;
		pod->jointCount = skinPod->joints.size();
		animationIndex++;
	}
}


} // namespace gltfutl
