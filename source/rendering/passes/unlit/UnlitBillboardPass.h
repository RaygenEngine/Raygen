#pragma once
#include "rendering/StaticPipeBase.h"
#include "rendering/wrappers/Buffer.h"

namespace vl {
struct UnlitBillboardPass : public StaticPipeBase {

	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;

	UnlitBillboardPass();

private:
	RBuffer m_rectangleVertexBuffer;
};

} // namespace vl
