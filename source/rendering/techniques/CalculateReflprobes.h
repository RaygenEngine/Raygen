#pragma once

struct SceneRenderDesc;

namespace vl {
struct CalculateReflprobes {

	static void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
