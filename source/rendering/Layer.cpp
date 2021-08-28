#include "Layer.h"

#include "assets/GpuAssetManager.h"
#include "assets/AssetRegistry.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
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

	Instance = new Instance_(Platform::GetVulkanExtensions(), Platform::GetMainHandle());
	Device = new Device_(Instance->physicalDevices[0]);
	CmdPoolManager = new CmdPoolManager_();
	GpuResources::Init();
	GpuAssetManager = new GpuAssetManager_();
	DescriptorLayouts = new DescriptorLayouts_();
	PassLayouts = new PassLayouts_();
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
	rvk::Shapes::DeinitShapes();

	delete Renderer;
	delete Pathtracer;
	delete RtxRenderer;
	delete m_swapOutput;
	delete m_mainScene;
	StaticPipes::DestroyAll();
	delete PassLayouts;
	delete DescriptorLayouts;
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

	m_swapOutput->OnPreRender();

	m_currentFrame = (m_currentFrame + 1) % c_framesInFlight;
	auto& currentCmdBuffer = m_cmdBuffer[m_currentFrame];

	{
		PROFILE_SCOPE(Renderer);

		// wait until previous frame fence of index m_currentFrame is signaled
		(void)Device->waitForFences(m_frameFence[m_currentFrame].get(), true, UINT64_MAX);
		(void)Device->resetFences(m_frameFence[m_currentFrame].get());

		m_mainScene->UploadDirty(m_currentFrame);
		m_mainScene->forceUpdateAccel = false;
	}

	// swap images and signal when image is available after swapping
	m_swapOutput->SwapImages(m_imageAvailSem[m_currentFrame].get());

	currentCmdBuffer.begin();
	{
		COMMAND_SCOPE(currentCmdBuffer, "Recorded Commands");
		m_renderer->RecordCmd(currentCmdBuffer, m_mainScene->GetRenderDesc(m_currentFrame), *m_swapOutput);
	}
	currentCmdBuffer.end();

	// wait for present image to be availabe during the color attachment output stage,
	// signal that rendering is done,
	// signal frame fence of index m_currentFrame
	currentCmdBuffer.submit(m_imageAvailSem[m_currentFrame].get(), vk::PipelineStageFlagBits::eColorAttachmentOutput,
		m_renderFinishedSem[m_currentFrame].get(), m_frameFence[m_currentFrame].get());

	// wait until rendering is done and then present
	m_swapOutput->Present(m_renderFinishedSem[m_currentFrame].get());
}
} // namespace vl
