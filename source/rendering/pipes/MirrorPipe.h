#pragma once
#include "rendering/pipes/StaticPipeBase.h"
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

namespace vl {
struct MirrorPipe : public StaticRaytracingPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	// DOC: parameters
	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const vk::Extent3D& extent,
		vk::DescriptorSet mirrorImageStorageDescSet, int32 bounces) const;
};
} // namespace vl
