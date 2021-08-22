#pragma once

#include "engine/Listener.h"

namespace vl {
class OutputPassBase;
}
struct SceneRenderDesc;

namespace vl {
class RendererBase : public Listener {
public:
	virtual void ResizeBuffers(uint32 width, uint32 height) = 0;
	virtual void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) = 0;
	virtual InFlightResources<vk::ImageView> GetOutputViews() const = 0;

	virtual ~RendererBase() = default;
};
} // namespace vl
