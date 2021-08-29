#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct AccumulationPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent, vk::DescriptorSet inputOutputStorageImages,
		int32 iteration) const;

	// TODO: this is an example of how to create specialized local groups in a compute shader. Use this to fix issues
	// with images but not in a static pipe - use automatic recompile when needed
	struct SpecializationData {
		uint32 sizeX;
		uint32 sizeY;
	} specializationData;
};
} // namespace vl
