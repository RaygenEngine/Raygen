#pragma once

struct SceneRenderDesc;

namespace vl {
struct CalculateReflprobes {

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;
};

} // namespace vl
