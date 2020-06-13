#include "pch.h"
#include "GBuffer.h"

#include "engine/console/ConsoleVariable.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/VulkanUtl.h"

ConsoleVariable<int32> console_rCullMode("r.culling", static_cast<int32>(vk::CullModeFlagBits::eBack));

namespace vl {
GBuffer::GBuffer(uint32 width, uint32 height)
{
	descSet = Layouts->gBufferDescLayout.GetDescriptorSet();

	auto initAttachment
		= [&](const std::string& name, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout finalLayout) {
			  auto att = std::make_unique<RImageAttachment>(name, width, height, format, vk::ImageTiling::eOptimal,
				  vk::ImageLayout::eUndefined, usage | vk::ImageUsageFlagBits::eSampled,
				  vk::MemoryPropertyFlagBits::eDeviceLocal);
			  att->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, finalLayout);
			  return att;
		  };

	for (size_t i = 0; i < 5; ++i) {
		attachments[i] = initAttachment(attachmentNames[i], colorAttachmentFormats[i],
			vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::Format depthFormat = Device->pd->FindDepthFormat();

	attachments[GDepth] = initAttachment(attachmentNames[GDepth], depthFormat,
		vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	std::array views = { attachments[GPosition]->GetView(), attachments[GNormal]->GetView(), //
		attachments[GAlbedo]->GetView(), attachments[GSpecular]->GetView(), attachments[GEmissive]->GetView(),
		attachments[GDepth]->GetView() };

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(Layouts->gbufferPass.get()) //
		.setAttachmentCount(static_cast<uint32>(views.size()))
		.setPAttachments(views.data())
		.setWidth(width)
		.setHeight(height)
		.setLayers(1);

	framebuffer = Device->createFramebufferUnique(createInfo);

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	// CHECK: update descriptor set (is this once?)
	for (uint32 i = 0; i < GCount; ++i) {

		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(attachments[i]->GetView())
			.setSampler(quadSampler);

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite
			.setDstSet(descSet) //
			.setDstBinding(i)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}

void GBuffer::TransitionForWrite(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	auto recordTransition = [&](auto& attachment) {
		auto target = !attachment->IsDepth() ? vk::ImageLayout::eColorAttachmentOptimal
											 : vk::ImageLayout::eDepthStencilAttachmentOptimal;

		auto barrier = attachment->CreateTransitionBarrier(vk::ImageLayout::eShaderReadOnlyOptimal, target);

		vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::PipelineStageFlags destinationStage = GetPipelineStage(target);

		cmdBuffer->pipelineBarrier(
			sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
	};

	for (auto& att : attachments) {
		recordTransition(att);
	}
}

} // namespace vl
