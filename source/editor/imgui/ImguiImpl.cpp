#include "pch.h"
#include "ImguiImpl.h"

#include "assets/PodIncludes.h"
#include "editor/imgui/ImGuizmo.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Engine.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "reflection/PodTools.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/Renderer.h"
#include "rendering/Swapchain.h"

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_vulkan.h>
#include <glfw/glfw3.h>

namespace imguistyle {
void AddLargeAssetIconsFont(ImFontAtlas* atlas)
{
	static ImVector<ImWchar> ranges;
	ImFontAtlas::GlyphRangesBuilder builder;


	podtools::ForEachPodType([&]<typename PodT>() {
		const ReflClass& cl = PodT::StaticClass();
		builder.AddText(U8(cl.GetIcon()));
	});
	builder.AddText(U8(FA_FOLDER));
	builder.BuildRanges(&ranges); // Build the final result (ordered ranges with all the unique characters submitted)


	ImguiImpl::s_AssetIconFont
		= atlas->AddFontFromFileTTF("engine-data/fonts/Font-Awesome-5-Free-Solid-900.ttf", 60.f, nullptr, ranges.Data);
}


inline void CorporateStyle()
{
	static ConsoleVarFunc<float> brighness{ "e.theme.textBrightness", &CorporateStyle, 1.f };

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	int is3D = 1;

	colors[ImGuiCol_Text] = ImVec4(brighness, brighness, brighness, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
	colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
	colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	style.PopupRounding = 3;

	style.WindowPadding = ImVec2(4, 4);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(6, 2);

	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = static_cast<float>(is3D);

	style.WindowRounding = 3;
	style.ChildRounding = 3;
	style.FrameRounding = 3;
	style.ScrollbarRounding = 2;
	style.GrabRounding = 3;

#ifdef IMGUI_HAS_DOCK
	style.TabBorderSize = static_cast<float>(is3D);
	style.TabRounding = 3;

	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
#endif
}

void SetStyleCustom()
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
}

void SetColors()
{
	static ConsoleVarFunc<int32> theme{ "e.theme", &SetColors, 2 };


	if (theme.Get() == 2) {
		CorporateStyle();
		return;
	}

	SetStyleCustom();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.98f;
	ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.89f, 0.98f);
	ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 0.8f);

	if (theme.Get() == 0) {
		return;
	}

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.70f, 0.69f, 0.67f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.77f, 0.75f, 0.70f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.57f, 0.55f, 0.53f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.72f, 0.71f, 0.65f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.72f, 0.71f, 0.65f, 0.40f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.37f, 0.37f, 0.37f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.54f, 0.54f, 0.54f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.79f, 0.56f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.78f, 0.57f, 0.78f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.71f, 0.44f, 0.78f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.75f, 0.72f, 0.56f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.33f, 0.33f, 0.33f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.94f, 0.87f, 0.74f, 0.42f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.92f, 0.84f, 0.65f, 0.72f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.80f, 0.75f, 0.50f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.67f, 0.65f, 0.57f, 0.78f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.76f, 0.80f, 0.84f, 0.93f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.60f, 0.73f, 0.88f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.92f, 0.93f, 0.94f, 0.99f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.74f, 0.82f, 0.91f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.92f, 0.84f, 0.65f, 0.40f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}


void SetStyle()
{


	// Static this, needs to be valid for as long as imgui is used.
	static const ImWchar ranges[] = { 0x0007, 0x00FF, 0 };


	auto& io = ImGui::GetIO();
	ImFontConfig fontConfig{};
	fontConfig.RasterizerMultiply = 1.1f;
	fontConfig.GlyphOffset = ImVec2(0.f, -1.f);
	ImguiImpl::s_EditorFont
		= io.Fonts->AddFontFromFileTTF("engine-data/fonts/OpenSans-SemiBold.ttf", 18.f, &fontConfig, ranges);


	static const ImWchar faRange[] = { 0xf000, 0xF941, 0 };
	ImFontConfig faConfig{};
	faConfig.MergeMode = true;
	faConfig.PixelSnapH = true;
	faConfig.GlyphMinAdvanceX = 12.f;
	faConfig.GlyphMaxAdvanceX = 12.f;
	io.Fonts->AddFontFromFileTTF("engine-data/fonts/Font-Awesome-5-Free-Solid-900.ttf", 15.0f, &faConfig, faRange);


	ImguiImpl::s_CodeFont
		= io.Fonts->AddFontFromFileTTF("engine-data/fonts/SourceCodePro-Semibold.ttf", 18.f, nullptr, ranges);

	AddLargeAssetIconsFont(io.Fonts);

	io.FontAllowUserScaling = true;
	io.Fonts->Build();

	SetColors();
}
} // namespace imguistyle


void InitVulkan()
{
	auto physDev = vl::Device->pd;
	auto& device = *vl::Device;

	ImGui_ImplVulkan_InitInfo init = {};
	init.Instance = *vl::Instance;
	init.PhysicalDevice = *physDev;
	init.Device = device;
	init.QueueFamily = vl::Device->graphicsQueue.familyIndex;
	init.Queue = vl::Device->graphicsQueue;
	init.PipelineCache = VK_NULL_HANDLE;
	init.DescriptorPool = vl::GpuResources->descPools.GetImguiPool();
	init.ImageCount = vl::Swapchain->GetImageCount();
	init.MinImageCount = vl::Swapchain->GetImageCount();
	init.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init, vl::Swapchain->GetRenderPass());

	// CHECK: which buffer
	auto cmdBuffer = vl::Device->graphicsCmdBuffer;

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

	vl::Device->graphicsQueue.submit(1, &end_info, {});
	vl::Device->graphicsQueue.waitIdle();

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}


void ImguiImpl::InitContext()
{
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	imguistyle::SetStyle();

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	// ImGui::GetIO().ConfigViewportsNoDecoration = false;

	ImGui_ImplGlfw_InitForVulkan(Platform::GetMainHandle(), true);
	ImGui::GetIO().IniFilename = "EditorImgui.ini";
	InitVulkan();
}

void ImguiImpl::NewFrame()
{

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
}

void ImguiImpl::CleanupContext()
{
	vl::Device->waitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


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

void ImguiImpl::RenderVulkan(vk::CommandBuffer* drawCommandBuffer)
{
	PROFILE_SCOPE(Editor);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *drawCommandBuffer);
}
