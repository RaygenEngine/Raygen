#include "Layer.h"

#include "assets/GpuAssetManager.h"
#include "assets/AssetRegistry.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/VkCoreIncludes.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/output/SwapchainOutputPass.h"
#include "rendering/Pathtracer.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/Renderer.h"
#include "rendering/RtxRenderer.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/VulkanLoader.h"


namespace vl {

Layer_::Layer_()
{
	VulkanLoader::InitLoaderBase();

	// TODO: use static init / destroy scheme?
	Instance = new Instance_(Platform::GetVulkanExtensions(), Platform::GetMainHandle());
	Device = new Device_(Instance->physicalDevices[0]);
	CmdPoolManager = new CmdPoolManager_();
	GpuResources::Init();
	GpuAssetManager = new GpuAssetManager_();
	Layouts = new Layouts_();
	rvk::Shapes::InitShapes();
	StaticPipes::InitRegistered();
	m_swapOutput = new SwapchainOutputPass();
	m_mainScene = new Scene();

	Renderer = new Renderer_();
	Pathtracer = new Pathtracer_();
	RtxRenderer = new RtxRenderer_();

	m_currentRasterizer = m_renderer = RtxRenderer;
	m_swapOutput->SetAttachedRenderer(m_renderer);

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		m_renderFinishedSem[i] = Device->createSemaphoreUnique({});
		m_imageAvailSem[i] = Device->createSemaphoreUnique({});
		m_frameFence[i] = Device->createFenceUnique({ vk::FenceCreateFlagBits::eSignaled });

		DEBUG_NAME(m_renderFinishedSem[i], "Renderer Finished" + std::to_string(i));
		DEBUG_NAME(m_imageAvailSem[i], "Image Available" + std::to_string(i));
	}

	m_cmdBuffer = InFlightCmdBuffers<Graphics>(vk::CommandBufferLevel::ePrimary);
} // namespace vl

Layer_::~Layer_()
{
	// TODO: use static init / destroy scheme?
	rvk::Shapes::DeinitShapes();

	delete Renderer;
	delete Pathtracer;
	delete RtxRenderer;
	delete m_swapOutput;
	delete m_mainScene;
	StaticPipes::DestroyAll();
	delete Layouts;
	delete GpuAssetManager;
	GpuResources::Destroy();

	m_frameFence = {};
	m_renderFinishedSem = {};
	m_imageAvailSem = {};
	m_cmdBuffer = {};

	delete CmdPoolManager;
	delete Device;
	delete Instance;
}

void Layer_::DrawFrame()
{
	PROFILE_SCOPE(Renderer);

	if (Input.IsJustPressed(Key::Tab)) [[unlikely]] {
		if (Input.IsDown(Key::Ctrl)) {
			if (m_renderer != Pathtracer) {
				m_currentRasterizer = m_renderer;
				m_renderer = Pathtracer;
			}
			else {
				m_renderer = m_currentRasterizer;
			}
		}
		else {
			if (m_renderer == RtxRenderer) {
				m_renderer = Renderer;
			}
			else {
				m_renderer = RtxRenderer;
			}
		}
		Device->waitIdle();
		m_swapOutput->SetAttachedRenderer(m_renderer);
		Device->waitIdle();
	}

	// DOC:
	if (!AssetRegistry::GetGpuUpdateRequests().empty()) {
		m_mainScene->forceUpdateAccel = true;
	}

	GpuAssetManager->ConsumeAssetUpdates();
	m_mainScene->ConsumeCmdQueue();

	if (!m_swapOutput->ShouldRenderThisFrame()) [[unlikely]] {
		return;
	}

	m_swapOutput->OnPreRender();

	m_currentFrame = (m_currentFrame + 1) % c_framesInFlight;
	auto& currentCmdBuffer = m_cmdBuffer[m_currentFrame];

	{
		PROFILE_SCOPE(Renderer);

		(void)Device->waitForFences({ *m_frameFence[m_currentFrame] }, true, UINT64_MAX);
		(void)Device->resetFences({ *m_frameFence[m_currentFrame] });

		m_mainScene->UploadDirty(m_currentFrame);
		m_mainScene->forceUpdateAccel = false;
	}

	uint32 imageIndex;

	// TODO: could this be a swapOut func?
	(void)Device->acquireNextImageKHR(
		m_swapOutput->GetSwapchain(), UINT64_MAX, { m_imageAvailSem[m_currentFrame].get() }, {}, &imageIndex);

	m_swapOutput->SetOutputImageIndex(imageIndex);


	currentCmdBuffer.begin();
	{
		CMDSCOPE_BEGIN(currentCmdBuffer, "Render frame");
		m_renderer->DrawFrame(currentCmdBuffer, m_mainScene->GetRenderDesc(m_currentFrame), *m_swapOutput);
		CMDSCOPE_END(currentCmdBuffer);
	}
	currentCmdBuffer.end();


	std::array<vk::PipelineStageFlags, 1> waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::array waitSems = { *m_imageAvailSem[m_currentFrame] };
	std::array signalSems = { *m_renderFinishedSem[m_currentFrame] };

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphores(waitSems) //
		.setWaitDstStageMask(waitStage)
		.setSignalSemaphores(signalSems);

	currentCmdBuffer.submit(submitInfo, m_frameFence[m_currentFrame].get());


	// TODO: could present be a swapOut func?
	vk::SwapchainKHR swapchain = m_swapOutput->GetSwapchain();

	vk::PresentInfoKHR presentInfo{};
	presentInfo //
		.setWaitSemaphores(m_renderFinishedSem[m_currentFrame].get())
		.setSwapchains(swapchain)
		.setImageIndices(imageIndex);

	(void)CmdPoolManager->presentQueue.presentKHR(presentInfo);
}
} // namespace vl
