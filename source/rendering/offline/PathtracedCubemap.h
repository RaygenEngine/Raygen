#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

struct SceneReflprobe;
struct SceneRenderDesc;

namespace vl {

class PathtracedCubemap {
public:
	PathtracedCubemap(SceneReflprobe* rp);

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, uint32 resolution);
	void Resize(const RCubemap& sourceCubemap, uint32 resolution);

private:
	SceneReflprobe* m_reflprobe{ nullptr };

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	std::array<vk::DescriptorSet, 6> m_faceDescSets;
	std::vector<vk::UniqueImageView> m_faceViews;

	void MakeRtPipeline();
	void CreateRtShaderBindingTable();
};
} // namespace vl
