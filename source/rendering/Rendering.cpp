#include "Rendering.h"

#include "rendering/Instance.h"
#include "rendering/Layer.h"
#include "rendering/output/SwapchainOutputPass.h"
#include "rendering/Pathtracer.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/Scene.h"

#include <imgui/examples/imgui_impl_vulkan.h>

ConsoleFunction<> cons_buildAll{ "s.structs.buildAll", []() { Rendering::GetMainScene()->BuildAll(); },
	"Builds all build-able scene structs." };

// TODO: uncomment, change name
// ConsoleFunction<> console_BuildAS{ "s.buildTestAccelerationStructure", []() {},
//	"Builds a top level acceleration structure, for debugging purposes, todo: remove" };

using namespace vl;

void Rendering::Init()
{
	Layer = new Layer_();
}

void Rendering::Destroy()
{
	delete Layer;
}

void Rendering::DrawFrame()
{
	Layer->DrawFrame();
}

Scene* Rendering::GetMainScene()
{
	return Layer->m_mainScene;
}

RendererBase* Rendering::GetActiveRenderer()
{
	return Layer->m_renderer;
}

void Rendering::ResetMainScene()
{
	delete Layer->m_mainScene;
	Layer->m_mainScene = new Scene();
}

bool Rendering::IsPathtracing()
{
	return Layer->m_renderer == Pathtracer;
}

void Rendering::Imgui_Prepare()
{
	auto& physDev = Device->pd;
	auto& device = *Device;

	ImGui_ImplVulkan_InitInfo init{};
	init.Instance = *Instance;
	init.PhysicalDevice = physDev;
	init.Device = device;
	init.QueueFamily = CmdPoolManager->graphicsQueue.family.index;
	init.Queue = CmdPoolManager->graphicsQueue;
	init.PipelineCache = VK_NULL_HANDLE;
	init.DescriptorPool = GpuResources::GetImguiPool();
	init.ImageCount = c_framesInFlight;
	init.MinImageCount = c_framesInFlight;
	init.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&init, Layer->m_swapOutput->GetRenderPass());

	{
		ScopedOneTimeSubmitCmdBuffer<Graphics> cmdBuffer{};
		ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
	}

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Rendering::Imgui_Shutdown()
{
	Device->waitIdle();
	ImGui_ImplVulkan_Shutdown();
}

void Rendering::Imgui_NewFrame()
{
	ImGui_ImplVulkan_NewFrame();
}

void Rendering::Imgui_DrawFrame(ImDrawData* drawData, vk::CommandBuffer* drawCommandBuffer)
{
	ImGui_ImplVulkan_RenderDrawData(drawData, *drawCommandBuffer);
}

void Rendering::SwapRenderingMode()
{
	if (IsPathtracing()) {
		Layer->m_renderer = Layer->m_currentRasterizer;
	}
	else {
		Layer->m_currentRasterizer = Layer->m_renderer;
		Layer->m_renderer = Pathtracer;
	}

	Device->waitIdle();
	Layer->m_swapOutput->SetAttachedRenderer(Layer->m_renderer);
	Device->waitIdle();
}
