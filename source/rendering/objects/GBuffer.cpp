#include "pch.h"
#include "rendering/objects/GBuffer.h"

#include "engine/Logger.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/Device.h"

namespace vl {
GBuffer::GBuffer(uint32 width, uint32 height)
{
	auto initAttachment = [&](auto& att, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout finalLayout) {
		att = std::make_unique<Image2D>(width, height, format, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
			usage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
		att->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, finalLayout);
	};

	for (size_t i = 0; i < 5; ++i) {
		initAttachment(attachments[i], colorAttachmentFormats[i], vk::ImageUsageFlagBits::eColorAttachment,
			vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::Format depthFormat = Device->pd->FindDepthFormat();

	initAttachment(attachments[GDepth], depthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

namespace {
	vk::PipelineStageFlags GetPipelineStage(vk::ImageLayout imL)
	{
		switch (imL) {
			case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
			case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
			case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits::eFragmentShader;
			case vk::ImageLayout::eTransferDstOptimal: return vk::PipelineStageFlagBits::eTransfer;
			case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
			default: LOG_ABORT("Unsupported");
		}
	}
} // namespace

void GBuffer::TransitionForAttachmentWrite(vk::CommandBuffer cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	auto recordTransition = [&](auto& attachment, vk::ImageLayout target = vk::ImageLayout::eColorAttachmentOptimal) {
		auto barrier = attachment->CreateTransitionBarrier(vk::ImageLayout::eShaderReadOnlyOptimal, target);

		vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::PipelineStageFlags destinationStage = GetPipelineStage(target);

		cmdBuffer.pipelineBarrier(
			sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
	};

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	for (size_t i = 0; i < 5; ++i) {
		recordTransition(attachments[i]);
	}
	recordTransition(attachments[GDepth], vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

std::array<vk::ImageView, 6> GBuffer::GetViewsArray()
{
	return { attachments[GPosition]->GetView(), attachments[GNormal]->GetView(), attachments[GAlbedo]->GetView(),
		attachments[GSpecular]->GetView(), attachments[GEmissive]->GetView(), attachments[GDepth]->GetView() };
}
} // namespace vl
