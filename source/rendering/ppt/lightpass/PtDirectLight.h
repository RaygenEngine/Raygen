#pragma once

#include "rendering/ppt/PtBase.h"

namespace vl {
class PtDirectLight : public PtBase_SinglePipeline {
public:
	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, uint32 frameIndex) override;
};
} // namespace vl
