#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {


struct PrefilteredMapCalculation : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

public:
	PrefilteredMapCalculation();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneReflprobe& rp) const;

private:
	RBuffer m_cubeVertexBuffer;
};
} // namespace vl
