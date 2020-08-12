#pragma once

#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/RDescriptorLayout.h"

namespace vl {
class PtDebug : public PtBase_SinglePipeline {
public:
	PtDebug();

	RDescriptorLayout descLayout;
	FrameArray<vk::DescriptorSet> descSet;

	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) override;
};

inline PtDebug* ptDebugObj{ nullptr };
} // namespace vl
