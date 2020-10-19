#pragma once
#include "rendering/StaticPipeBase.h"
#include "rendering/wrappers/Buffer.h"

namespace vl {

struct ReflprobeBlend : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;

	ReflprobeBlend();

private:
	void MakeSphere(int32 sectorCount, int32 stackCount, float radius = 1.0f);
	RBuffer m_sphereVertexBuffer;

	struct indices {
		RBuffer buffer;
		uint32 count;
	} m_sphereIndexBuffer;
};

} // namespace vl
