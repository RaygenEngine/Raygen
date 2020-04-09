#pragma once
#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/objects/Buffer.h"
#include "rendering/Device.h"

struct SceneGeometry {
	glm::mat4 transform;
	vl::GpuHandle<Mesh> model;

	GeometryNode* node;

	void Allocate(uint32 size){};
};

struct SceneCamera {
	struct Ubo {
		glm::vec4 position;
		glm::mat4 viewProj;
	} ubo;

	std::vector<vk::DescriptorSet> descSets;
	std::vector<UniquePtr<vl::Buffer<Ubo>>> buffers;

	void Upload(uint32 i)
	{
		buffers[i]->UploadData(ubo);

		vk::DescriptorBufferInfo bufferInfo{};

		bufferInfo
			.setBuffer(*buffers[i]) //
			.setOffset(0u)
			.setRange(sizeof(Ubo));
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(descSets[i]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}

	SceneCamera(uint32 size);
};

struct SceneSpotlight {
	struct Ubo {
		glm::vec4 position;
		glm::vec4 forward;

		// Lightmap
		glm::mat4 viewProj{};
		glm::vec4 color{};

		float intensity{};

		float near_{};
		float far_{};

		float outerCutOff{};
		float innerCutOff{};
	} ubo;

	std::vector<vk::DescriptorSet> descSets;
	std::vector<UniquePtr<vl::Buffer<Ubo>>> buffers;

	void Upload(uint32 i)
	{
		buffers[i]->UploadData(ubo);

		vk::DescriptorBufferInfo bufferInfo{};

		bufferInfo
			.setBuffer(*buffers[i]) //
			.setOffset(0u)
			.setRange(sizeof(Ubo));
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(descSets[i]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}

	bool isDirty{ true };

	SceneSpotlight(uint32 size);
};

template<typename T>
concept CSceneElem
	= std::is_same_v<SceneGeometry, T> || std::is_same_v<SceneCamera, T> || std::is_same_v<SceneSpotlight, T>;
