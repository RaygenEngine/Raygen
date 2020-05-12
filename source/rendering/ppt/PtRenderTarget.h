#pragma once
#include "rendering/objects/ImageAttachment.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class PtRenderTarget {

	vk::UniqueFramebuffer m_framebuffer;
	UniquePtr<ImageAttachment> m_attachment;

public:
	PtRenderTarget();
};

} // namespace vl
