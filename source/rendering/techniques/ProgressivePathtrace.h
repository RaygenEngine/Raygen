#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {
struct ProgressivePathtrace {
	ProgressivePathtrace();

	RImage2D progressive;
	vk::DescriptorSet progressiveDescSet;

	RBuffer viewer;
	vk::DescriptorSet viewerDescSet;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 iteration);
	void Resize(vk::Extent2D extent);
	void UpdateViewer(const glm::mat4& viewInv, const glm::mat4& projInv, float offset);
};

} // namespace vl
