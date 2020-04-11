#include "pch.h"
#include "SceneStructs.h"

#include "rendering/scene/Scene.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/VulkanUtl.h"
#include "rendering/Layouts.h"

SceneCamera::SceneCamera(uint32 size)
{
	for (uint32 i = 0; i < size; ++i) {
		descSets.push_back(vl::Layouts->cameraDescLayout.GetDescriptorSet());
		buffers.emplace_back(std::make_unique<vl::Buffer<Ubo>>(vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		isDirty.push_back(true);
	}
}

void SceneSpotlight::Upload()
{
	buffers[vl::Renderer_::currentFrame]->UploadData(ubo);

	{
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

	auto quadSampler = vl::GpuAssetManager->GetDefaultSampler();

	if (shadowmap.HasBeenGenerated()) {

		auto& atts = shadowmap.GetAttachments();

		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(atts[0]->GetView())
			.setSampler(quadSampler);

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite
			.setDstSet(descSets[vl::Renderer_::currentFrame]) //
			.setDstBinding(1u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}

void SceneSpotlight::PrepareShadowmap(vk::RenderPass renderPass, uint32 width, uint32 height)
{
	auto initAttachment = [&](const std::string& name, vk::Format format, vk::ImageUsageFlags usage,
							  vk::ImageLayout finalLayout, bool isDepth) {
		auto att = std::make_unique<vl::ImageAttachment>(name, width, height, format, vk::ImageLayout::eUndefined,
			usage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, isDepth);
		att->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, finalLayout);
		return att;
	};

	vk::Format depthFormat = vl::Device->pd->FindDepthFormat();

	shadowmap.AddAttachment(initAttachment("shadow", depthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, true));

	shadowmap.Generate(renderPass);
}

SceneSpotlight::SceneSpotlight(uint32 size)
{
	for (uint32 i = 0; i < size; ++i) {
		descSets.push_back(vl::Layouts->spotlightDescLayout.GetDescriptorSet());
		buffers.emplace_back(std::make_unique<vl::Buffer<Ubo>>(vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		isDirty.push_back(true);
	}
}

void SceneSpotlight::TransitionForAttachmentWrite(vk::CommandBuffer* cmdBuffer)

{
	shadowmap.TransitionForAttachmentWrite(cmdBuffer);
}
