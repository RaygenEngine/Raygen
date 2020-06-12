#pragma once
#include "rendering/objects/RImageAttachment.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class PtRenderTarget {

	vk::UniqueFramebuffer m_framebuffer;
	UniquePtr<RImageAttachment> m_attachment;

public:
	PtRenderTarget();
};

} // namespace vl
