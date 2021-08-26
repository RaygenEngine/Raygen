#include "RendererBase.h"

#include "rendering/wrappers/Framebuffer.h"

using namespace vl;

void RendererBase::RegisterDebugAttachment(vl::RImage& attachment)
{
	m_debugAttachments.emplace_back(attachment.GetDebugDescriptor(), attachment.name.c_str(),
		glm::ivec2{ attachment.extent.width, attachment.extent.height });
}

void RendererBase::RegisterDebugAttachment(vl::RFramebuffer& framebuffer)
{
	for (auto& att : framebuffer.ownedAttachments) {
		RegisterDebugAttachment(att);
	}
}
