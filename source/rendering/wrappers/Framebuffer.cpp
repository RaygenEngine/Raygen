#include "pch.h"
#include "Framebuffer.h"

#include "rendering/Device.h"

namespace {
bool IsEqual(const vk::Extent3D lhs, const vk::Extent2D rhs)
{
	return lhs.height == rhs.height && lhs.width == rhs.width;
}
} // namespace

namespace vl {

void RFramebuffer::AddAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
	vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
	const std::string& name, vk::ImageLayout finalLayout)
{
	if (extent.width == 0 && extent.height == 0) {
		extent.width = width;
		extent.height = height;
	}

	CLOG_ABORT(extent.width != width || extent.height != height,
		"Attempting to add attachment with different dimensions to a Framebuffer");

	ownedAttachments.emplace_back(width, height, format, tiling, initialLayout, usage, properties, name);
	ownedAttachments.back().BlockingTransitionToLayout(initialLayout, finalLayout);

	attachmentViews.emplace_back(ownedAttachments.back()());
}

void RFramebuffer::AddExistingAttachment(const RImageAttachment& attachment)
{
	CLOG_ERROR(!IsEqual(attachment.extent, extent),
		"Incompatible sizes for attachment to framebuffer: Attachment name: {}", attachment.name);
	attachmentViews.emplace_back(attachment());
}

void RFramebuffer::Generate(vk::RenderPass compatibleRenderPass)
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to generate a Framebuffer that is already generated");


	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(compatibleRenderPass) //
		.setAttachmentCount(static_cast<uint32>(attachmentViews.size()))
		.setPAttachments(attachmentViews.data())
		.setWidth(extent.width)
		.setHeight(extent.height)
		.setLayers(1);

	handle = Device->createFramebufferUnique(createInfo);

	hasBeenGenerated = true;
}
} // namespace vl
