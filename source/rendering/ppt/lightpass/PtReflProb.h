#pragma once

#include "rendering/ppt/PtBase.h"

namespace vl {
class PtReflProb : public PtBase_SinglePipeline {
public:
	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, SceneRenderDesc<SceneCamera>& sceneDesc, uint32 frameIndex) override;
};
} // namespace vl
