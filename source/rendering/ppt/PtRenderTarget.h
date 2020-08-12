#pragma once
#include "rendering/wrappers/RImage.h"

namespace vl {
class PtRenderTarget {

	vk::UniqueFramebuffer m_framebuffer;
	RImageAttachment m_attachment;

public:
	PtRenderTarget();
};

} // namespace vl
