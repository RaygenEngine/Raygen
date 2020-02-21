
#include "editor/imgui/ImguiImpl.h"
#include "system/Engine.h"
#include <glfw/glfw3.h>
#include "renderer/VkSampleRenderer.h"

#include <imgui.h>
#include <examples/imgui_impl_vulkan.h>
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


	auto& io = ImGui::GetIO();

	ImguiImpl::s_EditorFont = io.Fonts->AddFontFromFileTTF("engine-data/fonts/UbuntuMedium.ttf", 15, nullptr, ranges);


	static const ImWchar faRange[] = { 0xf000, 0xF941, 0 };
	ImFontConfig faConfig;
	faConfig.MergeMode = true;
	faConfig.PixelSnapH = true;
	io.Fonts->AddFontFromFileTTF("engine-data/fonts/Font-Awesome-5-Free-Solid-900.ttf", 15.0f, &faConfig, faRange);


	ImguiImpl::s_CodeFont
		= io.Fonts->AddFontFromFileTTF("engine-data/fonts/SourceCodePro-Semibold.ttf", 18, nullptr, ranges);

	io.FontAllowUserScaling = true;
	io.Fonts->Build();

	/* WIP light theme
ImVec4* colors = ImGui::GetStyle().Colors;
colors[ImGuiCol_Text]                   = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
colors[ImGuiCol_WindowBg]               = ImVec4(0.70f, 0.69f, 0.67f, 1.00f);
colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
colors[ImGuiCol_PopupBg]                = ImVec4(0.68f, 0.66f, 0.61f, 1.00f);
colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
colors[ImGuiCol_FrameBg]                = ImVec4(0.57f, 0.54f, 0.50f, 1.00f);
colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.79f, 0.70f, 0.30f, 0.40f);
colors[ImGuiCol_FrameBgActive]          = ImVec4(1.00f, 0.98f, 0.55f, 0.67f);
colors[ImGuiCol_TitleBg]                = ImVec4(0.37f, 0.37f, 0.37f, 1.00f);
colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
colors[ImGuiCol_MenuBarBg]              = ImVec4(0.54f, 0.54f, 0.54f, 1.00f);
colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
colors[ImGuiCol_CheckMark]              = ImVec4(0.98f, 0.79f, 0.56f, 1.00f);
colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 0.78f, 0.57f, 0.78f);
colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.78f, 0.80f, 0.46f, 0.60f);
colors[ImGuiCol_Button]                 = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
colors[ImGuiCol_ButtonHovered]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
colors[ImGuiCol_ButtonActive]           = ImVec4(0.99f, 1.00f, 0.72f, 1.00f);
colors[ImGuiCol_Header]                 = ImVec4(0.33f, 0.33f, 0.33f, 0.31f);
colors[ImGuiCol_HeaderHovered]          = ImVec4(1.00f, 0.91f, 0.50f, 0.42f);
colors[ImGuiCol_HeaderActive]           = ImVec4(0.96f, 0.98f, 0.26f, 1.00f);
colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
colors[ImGuiCol_Tab]                    = ImVec4(0.76f, 0.80f, 0.84f, 0.93f);
colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
colors[ImGuiCol_TabActive]              = ImVec4(0.60f, 0.73f, 0.88f, 1.00f);
colors[ImGuiCol_TabUnfocused]           = ImVec4(0.92f, 0.93f, 0.94f, 0.99f);
colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.74f, 0.82f, 0.91f, 1.00f);
colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.22f);
colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);


	*/
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
	// ImGui::GetIO().ConfigViewportsNoDecoration = false;

	ImGui_ImplGlfw_InitForVulkan(Engine::GetMainWindow(), true);
	ImGui::GetIO().IniFilename = "EditorImgui.ini";
}

void ImguiImpl::NewFrame()
{

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
}
#include "system/profiler/ProfileScope.h"
void ImguiImpl::EndFrame()
{
	{
		PROFILE_SCOPE(Editor);
		ImGui::EndFrame();


		ImGui::Render();
	}
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
	PROFILE_SCOPE(Editor);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *drawCommandBuffer);
}
