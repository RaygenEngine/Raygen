#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct CubemapPrefilterPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, vk::DescriptorSet storageImageDescSet,
		vk::DescriptorSet environmentSamplerDescSet, const vk::Extent3D& extent, const glm::mat4& viewInv,
		const glm::mat4& projInv) const;
};
} // namespace vl
