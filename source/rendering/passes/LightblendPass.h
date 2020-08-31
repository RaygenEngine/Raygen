#pragma once
#include "rendering/scene/Scene.h"

namespace vl {
class LightblendPass {

public:
	static void RecordCmd(
		vk::CommandBuffer* cmdBuffer, vk::Viewport viewport, vk::Rect2D scissor, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
