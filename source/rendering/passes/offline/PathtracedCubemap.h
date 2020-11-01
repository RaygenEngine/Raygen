#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {

struct PtCubeInfo {
	uint32 resolution;
	glm::vec4 worldPos;
	float traceOffset;
	int32 samples;
	int32 bounces;
	vk::DescriptorSet faceArrayDescSet;
};

class PathtracedCubemap {
public:
	PathtracedCubemap();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const PtCubeInfo& info);

private:
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	void MakeRtPipeline();
	void CreateRtShaderBindingTable();
};
} // namespace vl
