#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct CubemapPrefilterPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent, vk::DescriptorSet storageImageDescSet,
		vk::DescriptorSet environmentSamplerDescSet, const glm::mat4& viewInv, const glm::mat4& projInv) const;
};
} // namespace vl
