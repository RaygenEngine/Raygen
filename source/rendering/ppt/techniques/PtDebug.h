#pragma once

#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/DescriptorSetLayout.h"

namespace vl {
class PtDebug : public PtBase_SinglePipeline {
public:
	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) override;
};
} // namespace vl
