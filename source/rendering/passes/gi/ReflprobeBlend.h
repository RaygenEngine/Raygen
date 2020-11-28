#pragma once
#include "rendering/StaticPipeBase.h"
#include "rendering/wrappers/Buffer.h"

namespace vl {

struct ReflprobeBlend : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;

private:
	RBuffer m_cubeVertexBuffer; // strip
};

} // namespace vl
