#pragma once
#include "rendering/pipes/StaticPipeBase.h"
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

namespace vl {
struct MirrorPipe : public StaticRaytracingPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
		vk::DescriptorSet mirrorImageStorageDescSet, const vk::Extent3D& extent) const;
};
} // namespace vl
