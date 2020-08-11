#pragma once

#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/RDescriptorLayout.h"

namespace vl {
class PtDebug : public PtBase_SinglePipeline {
public:
	PtDebug();

	RDescriptorLayout descLayout;
	std::array<vk::DescriptorSet, 3> descSet;

	void MakeLayout() override;
	void MakePipeline() override;
	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) override;
};

inline PtDebug* ptDebugObj{ nullptr };
} // namespace vl
