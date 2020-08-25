#include "pch.h"

#include "RenderPassLayout.h"

#include "rendering/Device.h"

namespace vl {

void RRenderPassLayout::Generate()
{

	std::vector<vk::AttachmentDescription> attachmentDescrs;
	std::vector<vk::SubpassDescription> subpassDescrs;

	for (auto& att : internalAttachments) {
		attachmentDescrs.emplace_back(att.descr);

		CLOG_ERROR(att.descr.finalLayout == vk::ImageLayout::eUndefined,
			"Found undefined image layout when generating layout.");
	}

	for (auto& subp : subpasses) {
		subpassDescrs.emplace_back(subp.descr);
	}

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachments(attachmentDescrs) //
		.setSubpasses(subpassDescrs)
		.setDependencies(subpassDependencies);

	compatibleRenderPass = Device->createRenderPassUnique(renderPassInfo);
}
} // namespace vl
