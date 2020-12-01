#pragma once

struct SceneRenderDesc;

namespace vl {
struct CalculateShadowmaps {

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;
};

} // namespace vl
