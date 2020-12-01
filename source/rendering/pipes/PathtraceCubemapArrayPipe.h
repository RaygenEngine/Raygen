#pragma once
#include "rendering/pipes/StaticPipeBase.h"
#include "rendering/wrappers/Buffer.h"

struct SceneIrragrid;

namespace vl {
struct PathtraceCubemapArrayPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrragrid& ig) const;

private:
	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;
};
} // namespace vl
