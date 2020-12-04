#pragma once

struct SceneRenderDesc;

namespace vl {
struct CalculateIrragrids {

	static void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
