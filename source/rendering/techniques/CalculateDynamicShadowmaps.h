#pragma once

#include "rendering/VkCoreIncludes.h"

struct SceneRenderDesc;

namespace vl {
struct CalculateDynamicShadowmaps {
	static void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
