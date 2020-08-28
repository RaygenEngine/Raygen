#pragma once

#include "engine/Listener.h"

namespace vl {
class Renderer_;


struct RenderTarget {
	// Request side:
	uint32 width{ 0 };
	uint32 height{ 0 };

	struct ViewData {
		glm::vec4 position;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProj;
		glm::mat4 viewInv;
		glm::mat4 projInv;
		glm::mat4 viewProjInv;
	} viewData;
};


// Base class for output passes.
// An output pass is the final step (or pass) of the renderer. It takes as input a single attachment in a predetermined
// form (RGBA32Sfloat currently) from the renderer and uses it to produce actual output.
// ie: Present to screen, Make screenshots, record video ...

class OutputPassBase : public Listener {
protected:
	Renderer_* m_renderer{ nullptr };

public:
	virtual void SetAttachedRenderer(Renderer_* renderer) { m_renderer = renderer; }


	// This will get called by the renderer itself when the underlying view is updated.
	virtual void OnViewsUpdated(InFlightResources<vk::ImageView> renderResultViews) = 0;

	virtual void RecordOutPass(vk::CommandBuffer cmdBuffer, uint32 frameIndex) = 0;

	virtual bool ShouldRenderThisFrame() = 0;

	// This will not run in a frame where ShouldRenderThisFrame reutrn returns false
	virtual void OnPreRender() = 0;

	virtual ~OutputPassBase() = default;
};

} // namespace vl
