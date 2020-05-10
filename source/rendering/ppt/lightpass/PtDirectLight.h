#pragma once

#include "rendering/ppt/PtBase.h"

namespace vl {
class PtDirectLight : public PtBase {
public:
	vk::UniquePipelineLayout m_pipelineLayout;
	vk::UniquePipeline m_pipeline;

	PtDirectLight();
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, uint32 frameIndex) override;
};
} // namespace vl
