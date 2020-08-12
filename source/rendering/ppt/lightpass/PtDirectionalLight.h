#pragma once

#include "rendering/ppt/PtBase.h"

namespace vl {
class PtDirectionalLight : public PtBase_SinglePipeline {
public:
	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet gbufferDescSet) override;
};
} // namespace vl
