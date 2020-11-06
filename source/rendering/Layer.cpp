#include "Layer.h"

#include "assets/GpuAssetManager.h"
#include "editor/EditorObject.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/output/SwapchainOutputPass.h"
#include "rendering/Renderer.h"
#include "rendering/StaticPipes.h"
#include "rendering/VulkanLoader.h"
#include "resource/GpuResources.h"

ConsoleFunction<> console_BuildAll{ "s.buildAll", []() { vl::Layer->mainScene->BuildAll(); },
	"Builds all build-able scene nodes" };
// TODO: uncomment, change name
// ConsoleFunction<> console_BuildAS{ "s.buildTestAccelerationStructure", []() {},
//	"Builds a top level acceleration structure, for debugging purposes, todo: remove" };

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
	StaticPipes::InitRegistered();
	swapOutput = new SwapchainOutputPass();
	mainScene = new Scene();
	Renderer = new Renderer_();

	swapOutput->SetAttachedRenderer(Renderer);
	Renderer->InitPipelines();

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
	delete Renderer;
	delete swapOutput;
	delete mainScene;
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
	// DOC:
	if (!AssetRegistry::GetGpuUpdateRequests().empty()) {
		mainScene->forceUpdateAccel = true;
	}

	GpuAssetManager->ConsumeAssetUpdates();
	mainScene->ConsumeCmdQueue();

	if (!swapOutput->ShouldRenderThisFrame()) [[unlikely]] {
		return;
	}

	swapOutput->OnPreRender();

	m_currentFrame = (m_currentFrame + 1) % c_framesInFlight;
	auto& currentCmdBuffer = m_cmdBuffer[m_currentFrame];

	{
		PROFILE_SCOPE(Renderer);

		Device->waitForFences({ *m_frameFence[m_currentFrame] }, true, UINT64_MAX);
		Device->resetFences({ *m_frameFence[m_currentFrame] });

		mainScene->UploadDirty(m_currentFrame);
		mainScene->forceUpdateAccel = false;
	}

	uint32 imageIndex;

	// TODO: could this be a swapOut func?
	Device->acquireNextImageKHR(
		swapOutput->GetSwapchain(), UINT64_MAX, { m_imageAvailSem[m_currentFrame].get() }, {}, &imageIndex);

	swapOutput->SetOutputImageIndex(imageIndex);


	currentCmdBuffer.begin();
	{
		Renderer->DrawFrame(currentCmdBuffer, mainScene->GetRenderDesc(m_currentFrame), *swapOutput);
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
	vk::SwapchainKHR swapchain = swapOutput->GetSwapchain();

	vk::PresentInfoKHR presentInfo{};
	presentInfo //
		.setWaitSemaphores(m_renderFinishedSem[m_currentFrame].get())
		.setSwapchains(swapchain)
		.setImageIndices(imageIndex);

	CmdPoolManager->presentQueue.presentKHR(presentInfo);
}

void Layer_::ResetMainScene()
{
	delete mainScene;
	mainScene = new Scene();
}
} // namespace vl
