#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

namespace vl {

class PathtracedCubemap {
public:
	PathtracedCubemap(GpuEnvironmentMap* envmapAsset, glm::vec3 position, uint32 res);

	void Calculate(vk::DescriptorSet sceneAsDescSet, vk::DescriptorSet sceneGeomDataDescSet,
		vk::DescriptorSet sceneSpotlightDescSet);

private:
	GpuEnvironmentMap* m_envmapAsset;
	uint32 m_resolution;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	std::array<vk::DescriptorSet, 6> m_faceDescSets;
	std::array<RImageAttachment, 6> m_faceAttachments;

	std::array<glm::mat4, 6> m_viewMats;

	void MakeRtPipeline();
	void CreateRtShaderBindingTable();
	void CreateMatrices(const glm::vec3& reflprobePos);
	void CreateFaceAttachments();
	void EditPods();
};
} // namespace vl
