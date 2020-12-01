#pragma once

struct SceneRenderDesc;

namespace vl {
struct CalculateIrragrids {

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;
};

} // namespace vl
