#pragma once

#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/DescriptorSetLayout.h"

namespace vl {
class PtDebug : public PtBase_SinglePipeline {
public:
	PtDebug();

	RDescriptorSetLayout descLayout;
	InFlightResources<vk::DescriptorSet> descSet;

	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet gbufferDescSet) override;
};

inline PtDebug* ptDebugObj{ nullptr };
} // namespace vl
