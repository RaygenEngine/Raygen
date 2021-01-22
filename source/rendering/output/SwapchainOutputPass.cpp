#include "SwapchainOutputPass.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"

namespace vl {

SwapchainOutputPass::SwapchainOutputPass()
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_descSet[i] = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
	}

	OnWindowResize();
	OnViewportResize();
	RecompileCpyShader();

	Event::OnViewportUpdated.BindFlag(this, m_didViewportResize);
	Event::OnWindowResize.BindFlag(this, m_didWindowResize);
	Event::OnWindowMinimize.Bind(this, [&](bool isMinimized) { m_isMinimized = isMinimized; });
	Event::OnWindowFocus.Bind(this, [&](bool isFocused) { m_isFocused = isFocused; });
}

void SwapchainOutputPass::SetAttachedRenderer(RendererBase* renderer)
{
	m_renderer = renderer;
	OnViewportResize();
}

void SwapchainOutputPass::RecompileCpyShader()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/cpyhdr.shader");
	gpuShader.onCompile = [=]() {
		RecompileCpyShader();
	};

	m_pipelineLayout = rvk::makeLayoutNoPC({ Layouts->singleSamplerDescLayout.handle() });
	m_pipeline = rvk::makePostProcPipeline(gpuShader.shaderStages, *m_pipelineLayout, m_swapchain->renderPass());
}

void SwapchainOutputPass::OnViewsUpdated(InFlightResources<vk::ImageView> renderResultViews)
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		rvk::writeDescriptorImages(m_descSet[i], 0, { renderResultViews[i] });
	}
}

void SwapchainOutputPass::RecordOutPass(vk::CommandBuffer cmdBuffer, uint32 frameIndex)
{
	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(m_swapchain->renderPass()) //
		.setFramebuffer(m_swapchain->framebuffer(m_swapchainImageIndex));

	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(m_swapchain->extent);

	vk::ClearValue clearValue{};
	clearValue.setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	renderPassInfo
		.setClearValueCount(1u) //
		.setPClearValues(&clearValue);

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{
		auto& scissor = m_viewportRect;
		const float x = static_cast<float>(scissor.offset.x);
		const float y = static_cast<float>(scissor.offset.y);
		const float width = static_cast<float>(scissor.extent.width);
		const float height = static_cast<float>(scissor.extent.height);

		vk::Viewport viewport{};
		viewport
			.setX(x) //
			.setY(y)
			.setWidth(width)
			.setHeight(height)
			.setMinDepth(0.f)
			.setMaxDepth(1.f);

		cmdBuffer.setViewport(0, { viewport });
		cmdBuffer.setScissor(0, { scissor });

		// Copy hdr texture
		{
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());
			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, { m_descSet[frameIndex] }, nullptr);

			// big triangle
			cmdBuffer.draw(3u, 1u, 0u, 0u);
		}

		ImguiImpl::RenderVulkan(cmdBuffer);
	}
	cmdBuffer.endRenderPass();
}

void SwapchainOutputPass::OnPreRender()
{
	using namespace std::literals;
	if (!m_isFocused) {
		std::this_thread::sleep_for(100ms);
	}

	if (*m_didWindowResize) {
		OnWindowResize();
	}

	if (*m_didViewportResize) {
		OnViewportResize();
	}
}

bool SwapchainOutputPass::ShouldRenderThisFrame()
{
	return !m_isMinimized;
}

void SwapchainOutputPass::OnWindowResize()
{
	Device->waitIdle();

	m_swapchain.reset();
	m_swapchain = std::make_unique<RSwapchain>(Instance->surface);
}

void SwapchainOutputPass::OnViewportResize()
{
	m_viewportRect.extent = vk::Extent2D(g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y);
	m_viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	if (m_renderer) {
		m_renderer->ResizeBuffers(m_viewportRect.extent.width, m_viewportRect.extent.height);
		OnViewsUpdated(m_renderer->GetOutputViews());
	}
}

} // namespace vl
