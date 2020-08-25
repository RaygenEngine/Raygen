#include "pch.h"
#include "Framebuffer.h"

#include "rendering/Device.h"

namespace vl {

void RFramebuffer::AddAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
	vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
	const std::string& name, vk::ImageLayout finalLayout)
{
	if (extent.width == 0 && extent.height == 0) {
		extent.width = width;
		extent.height = height;
	}

	CLOG_ERROR(extent.width != width || extent.height != height,
		"Attempting to add attachment with different dimensions to a Framebuffer");

	attachments.emplace_back(width, height, format, tiling, initialLayout, usage, properties, name);
	attachments.back().BlockingTransitionToLayout(initialLayout, finalLayout);
}

void RFramebuffer::Generate(vk::RenderPass compatibleRenderPass)
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to generate a Framebuffer that is already generated");

	std::vector<vk::ImageView> views;
	std::for_each(attachments.begin(), attachments.end(), [&views](RImageAttachment& att) { views.push_back(att()); });

	auto extent = attachments[0].extent;

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(compatibleRenderPass) //
		.setAttachmentCount(static_cast<uint32>(views.size()))
		.setPAttachments(views.data())
		.setWidth(extent.width)
		.setHeight(extent.height)
		.setLayers(1);

	handle = Device->createFramebufferUnique(createInfo);

	hasBeenGenerated = true;
}
} // namespace vl
