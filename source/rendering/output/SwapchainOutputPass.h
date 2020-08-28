#pragma once

#include "rendering/output/OutputPassBase.h"
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/Swapchain.h"


namespace vl {
class SwapchainOutputPass : public OutputPassBase {


public:
	SwapchainOutputPass();

	virtual void SetAttachedRenderer(Renderer_* renderer) override;
	// This will get called by the renderer itself when the underlying view is updated.
	virtual void OnViewsUpdated(InFlightResources<vk::ImageView> renderResultViews) override;

	virtual void RecordOutPass(vk::CommandBuffer cmdBuffer, uint32 frameIndex) override;

	virtual bool ShouldRenderThisFrame() override;

	virtual void OnPreRender() override;

	[[nodiscard]] vk::SwapchainKHR GetSwapchain() const { return m_swapchain->handle(); }
	[[nodiscard]] vk::RenderPass GetRenderPass() const { return m_swapchain->renderPass(); }

	// Sets the temporary  ImageIndex given by the swapchain aquire image call
	void SetOutputImageIndex(uint32 swapchainImageIndex) { m_swapchainImageIndex = swapchainImageIndex; }

private:
	void RecompileCpyShader();

	void OnWindowResize();
	void OnViewportResize();

	InFlightResources<vk::DescriptorSet> m_descSet;


	UniquePtr<RSwapchain> m_swapchain;

	vk::UniquePipelineLayout m_pipelineLayout;
	vk::UniquePipeline m_pipeline;
	vk::Rect2D m_viewportRect{};

	BoolFlag m_didViewportResize;
	BoolFlag m_didWindowResize;

	uint32 m_swapchainImageIndex{};

	bool m_isMinimized{ false };
};

} // namespace vl
