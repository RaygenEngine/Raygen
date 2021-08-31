#include "SwapchainOutputPass.h"

#include "editor/Editor.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"

namespace {
enum TONEMAP_MODE : int32
{
	DEFAULT = 0,
	UNCHARTED,
	HEJLRICHARD,
	ACES,
};

struct PushConstant {
	float gamma;
	float exposure;
	int32 tonemapMode;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

SwapchainOutputPass::SwapchainOutputPass()
{
	for (size_t i = 0; i < c_framesInFlight; ++i) {
		m_descSet[i] = DescriptorLayouts->_1imageSampler.AllocDescriptorSet();
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

	m_pipelineLayout = rvk::makePipelineLayout<PushConstant>(
		{ DescriptorLayouts->_1imageSampler.handle() }, vk::ShaderStageFlagBits::eFragment);

	m_pipeline = rvk::makeGraphicsPipeline(gpuShader.shaderStages, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, m_pipelineLayout.get(), m_swapchain->renderPass(), 0u);
}

void SwapchainOutputPass::OnViewsUpdated(InFlightResources<vk::ImageView> renderResultViews)
{
	for (size_t i = 0; i < c_framesInFlight; ++i) {
		rvk::writeDescriptorImages(m_descSet[i], 0, { renderResultViews[i] });
	}
}

void SwapchainOutputPass::RecordOutPass(vk::CommandBuffer cmdBuffer, uint32 frameIndex)
{
	COMMAND_SCOPE(cmdBuffer, "Output Pass");

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(m_swapchain->renderPass()) //
		.setFramebuffer(m_swapchain->framebuffer(m_swapchain->imageIndex));

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

		{
			COMMAND_SCOPE(cmdBuffer, "Copy Hdr Texture");

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());
			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, { m_descSet[frameIndex] }, nullptr);

			static ConsoleVariable<float> cons_gamma{ "r.out.gamma", 2.2f, "Set the gamma correction." };
			static ConsoleVariable<float> cons_exposure{ "r.out.exposure", 1.0f, "Set the exposure." };
			static ConsoleVariable<TONEMAP_MODE> cons_tonemapMode{ "r.out.tonemap.mode", DEFAULT,
				"Set the tonemapping mode." };

			PushConstant pc{
				*cons_gamma,
				*cons_exposure,
				cons_tonemapMode,
			};

			cmdBuffer.pushConstants(
				m_pipelineLayout.get(), vk::ShaderStageFlagBits::eFragment, 0u, sizeof(PushConstant), &pc);

			// big triangle
			cmdBuffer.draw(3u, 1u, 0u, 0u);
		}

		Editor::Draw(&cmdBuffer);
	}
	cmdBuffer.endRenderPass();
}

void SwapchainOutputPass::OnPreRender()
{
	static ConsoleVariable<bool> cons_workInBackground{ "r.out.workInBackground", false,
		"Enable rendering even when window is unfocused. (Useful for progressive renderers)" };

	using namespace std::literals;
	if (!m_isFocused && !(*cons_workInBackground)) {
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
