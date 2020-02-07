
#include "editor/imgui/ImguiImpl.h"
#include "system/Engine.h"
#include <glfw/glfw3.h>
#include "renderer/renderers/vulkan/VkSampleRenderer.h"

#include <imgui.h>
#include <examples/imgui_impl_vulkan.h>
#include <examples/imgui_impl_win32.h>
#include <examples/imgui_impl_glfw.h>

namespace imguisyle {
void SetStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// If you want to edit this, use ImGui::ShowStyleEditor for an actual editor.

	style.WindowPadding = ImVec2(6.f, 3.f);
	style.FramePadding = ImVec2(3.f, 3.f);
	style.ItemSpacing = ImVec2(2.f, 2.f);
	style.ItemInnerSpacing = ImVec2(3.f, 3.f);
	style.TouchExtraPadding = ImVec2(3.f, 1.f);
	style.IndentSpacing = 14.f;
	style.ScrollbarSize = 15.f;
	// style.GrabMinSize default

	style.WindowBorderSize = 0.f;
	style.ChildBorderSize = 1.f;
	style.PopupBorderSize = 1.f;
	style.FrameBorderSize = 0.f;
	style.TabBorderSize = 0.f;

	style.WindowRounding = 0.f;
	style.ChildRounding = 0.f;
	style.FrameRounding = 0.f;
	style.PopupRounding = 0.f;
	style.ScrollbarRounding = 0.f;
	style.GrabRounding = 0.f;
	style.TabRounding = 0.f;


	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg].w = 0.98f;
	colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.89f, 0.98f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 0.8f);

	// Static this, needs to be valid for as long as imgui is used.
	static const ImWchar ranges[] = { 0x0007, 0x00FF, 0 };

	ImGui::GetIO().Fonts->AddFontFromFileTTF("engine-data/fonts/UbuntuMedium.ttf", 15, nullptr, ranges);
	ImGui::GetIO().Fonts->AddFontFromFileTTF("engine-data/fonts/UbuntuMonoRegular.ttf", 14, nullptr, ranges);

	ImGui::GetIO().Fonts->Build();
}
} // namespace imguisyle
void ImguiImpl::InitContext()
{
	if (!Engine::GetMainWindow()) {
		LOG_ERROR("Failed to load imgui, window not created yet. Please make a main window before imgui init.");
		return;
	}
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	imguisyle::SetStyle();

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::GetIO().ConfigViewportsNoDecoration = false;

	ImGui_ImplGlfw_InitForVulkan(Engine::GetMainWindow(), true);
	ImGui::GetIO().IniFilename = "EditorImgui.ini";
}

void ImguiImpl::NewFrame()
{

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
}

void ImguiImpl::EndFrame()
{
	ImGui::EndFrame();
	ImGui::Render();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void ImguiImpl::CleanupContext()
{
	// WIP
	// ImGui_ImplVulkan_Shutdown();
	// ImGui_ImplGlfw_Shutdown();

	// ImGui::DestroyContext();
}


void ImguiImpl::InitVulkan()
{
	auto& r = *Engine::GetRenderer();

	auto physDev = r.m_device->GetPhysicalDevice();

	ImGui_ImplVulkan_InitInfo init = {};
	init.Instance = r.m_instanceLayer->GetInstance();
	init.PhysicalDevice = *physDev;
	init.Device = *r.m_device.get();
	init.QueueFamily = physDev->GetBestGraphicsFamily().familyIndex;
	init.Queue = r.m_device->GetGraphicsQueue();
	init.PipelineCache = VK_NULL_HANDLE;
	init.DescriptorPool = r.m_descriptors->GetDescriptorPool();

	init.ImageCount = r.m_swapchain->GetImageCount();
	init.MinImageCount = r.m_swapchain->GetImageCount();
	init.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init, r.m_swapchain->GetRenderPass());


	auto cmdBuffer = r.m_device->GetTransferCommandBuffer();

	//	vkCall(vkResetCommandPool(m_device, m_commandPool, 0));

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	cmdBuffer.begin(begin_info);

	ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);

	vk::SubmitInfo end_info = {};
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &cmdBuffer;

	cmdBuffer.end();

	r.m_device->GetTransferQueue().submit(1, &end_info, {});
	r.m_device->waitIdle();

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}


void ImguiImpl::RenderVulkan(vk::CommandBuffer* drawCommandBuffer)
{
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *drawCommandBuffer);
}

#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>
#include <examples/imgui_impl_vulkan.cpp>
#include <examples/imgui_impl_glfw.cpp>
#include <misc/cpp/imgui_stdlib.cpp>
