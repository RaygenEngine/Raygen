#pragma once
#include "rendering/ppt/PtBase.h"
#include "rendering/scene/Scene.h"

namespace vl {
class PtLightBlend : public PtBase_SinglePipeline {
public:
	void MakeLayout() override;
	void MakePipeline() override;
	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) override;
};
} // namespace vl
