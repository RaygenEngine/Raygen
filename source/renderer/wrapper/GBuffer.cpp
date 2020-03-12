#include "pch.h"
#include "renderer/wrapper/GBuffer.h"

#include "renderer/wrapper/Device.h"
#include "engine/profiler/ProfileScope.h"
#include "engine/Logger.h"

GBuffer::GBuffer(uint32 width, uint32 height)
{
	position = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	normal = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	albedo = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	specular = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	emissive = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	vk::Format depthFormat = Device->pd->FindDepthFormat();

	depth = std::make_unique<Attachment>(width, height, depthFormat, vk::ImageLayout::eDepthStencilAttachmentOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
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

	auto recordTransition
		= [&](Attachment& attachment, vk::ImageLayout target = vk::ImageLayout::eColorAttachmentOptimal) {
			  auto barrier = attachment.image->CreateTransitionBarrier(vk::ImageLayout::eShaderReadOnlyOptimal, target);

			  vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);
			  vk::PipelineStageFlags destinationStage = GetPipelineStage(target);

			  cmdBuffer.pipelineBarrier(
				  sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
		  };

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);


	recordTransition(*position);
	recordTransition(*normal);
	recordTransition(*albedo);
	recordTransition(*specular);
	recordTransition(*emissive);
	recordTransition(*depth, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

std::array<vk::ImageView, 6> GBuffer::GetViewsArray()
{
	return { position->view.get(), normal->view.get(), albedo->view.get(), specular->view.get(), emissive->view.get(),
		depth->view.get() };
}
