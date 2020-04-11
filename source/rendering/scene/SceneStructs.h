#pragma once
#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/Device.h"
#include "rendering/objects/Buffer.h"
#include "rendering/objects/ImageAttachment.h"
#include "rendering/Renderer.h"
#include "rendering/resource/DescPoolAllocator.h"
#include "universe/nodes/geometry/GeometryNode.h"

struct SceneGeometry {
	glm::mat4 transform;
	vl::GpuHandle<Mesh> model;

	GeometryNode* node;
	std::vector<bool> isDirty;
};

struct SceneCamera {
	struct Ubo {
		glm::vec4 position;
		glm::mat4 viewProj;
	} ubo;

	std::vector<vk::DescriptorSet> descSets;
	std::vector<UniquePtr<vl::Buffer<Ubo>>> buffers;

	void Upload()
	{
		buffers[vl::Renderer_::currentFrame]->UploadData(ubo);

		vk::DescriptorBufferInfo bufferInfo{};

		bufferInfo
			.setBuffer(*buffers[vl::Renderer_::currentFrame]) //
			.setOffset(0u)
			.setRange(sizeof(Ubo));
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(descSets[vl::Renderer_::currentFrame]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}

	std::vector<bool> isDirty;

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

		float constantTerm{};
		float linearTerm{};
		float quadraticTerm{};
	} ubo;

	std::vector<vk::DescriptorSet> descSets;
	std::vector<UniquePtr<vl::Buffer<Ubo>>> buffers;

	vl::Framebuffer shadowmap;

	void Upload();

	void PrepareShadowmap(vk::RenderPass renderPass, uint32 width, uint32 height);


	std::vector<bool> isDirty;

	SceneSpotlight(uint32 size);

	void TransitionForAttachmentWrite(vk::CommandBuffer* cmdBuffer);
};

template<typename T>
concept CSceneElem
	= std::is_same_v<SceneGeometry, T> || std::is_same_v<SceneCamera, T> || std::is_same_v<SceneSpotlight, T>;
