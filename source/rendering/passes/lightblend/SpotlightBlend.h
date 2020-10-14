#pragma once
#include "rendering/scene/Scene.h" // TODO: fwd declare?

namespace vl {
struct SpotlightBlend {
	static vk::UniquePipelineLayout MakePipelineLayout();
	static vk::UniquePipeline MakePipeline();

	static void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
