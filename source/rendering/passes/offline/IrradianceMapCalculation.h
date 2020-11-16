#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct CalcIrrInfo {
	uint32 resolution;
	std::vector<vk::Framebuffer> faceFramebuffers;
	vk::DescriptorSet envmapDescSet;
};

struct IrradianceMapCalculation : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

public:
	IrradianceMapCalculation();

	void RecordPass(vk::CommandBuffer cmdBuffer, const CalcIrrInfo& info) const;

	RBuffer m_cubeVertexBuffer;
};
} // namespace vl
