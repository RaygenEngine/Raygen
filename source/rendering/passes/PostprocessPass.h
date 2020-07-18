#pragma once

namespace vl {
class PostprocessPass {
	vk::UniqueRenderPass m_renderPass;

public:
	PostprocessPass();
	void RecordCmd();
};

} // namespace vl
