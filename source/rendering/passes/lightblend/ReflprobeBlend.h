#pragma once

struct SceneRenderDesc;

namespace vl {

struct ReflprobeBlend {
	static vk::UniquePipelineLayout MakePipelineLayout();
	static vk::UniquePipeline MakePipeline();

	static void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
