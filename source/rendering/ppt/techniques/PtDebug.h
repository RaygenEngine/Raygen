#pragma once

#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/RDescriptorLayout.h"

namespace vl {
class PtDebug : public PtBase_SinglePipeline {
public:
	static inline RDescriptorLayout descLayout;
	static inline vk::DescriptorSet descSet[3];

	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, uint32 frameIndex) override;
};
} // namespace vl
