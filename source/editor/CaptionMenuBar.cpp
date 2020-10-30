#include "CaptionMenuBar.h"

#include "editor/imgui/ImEd.h"
#include "editor/EditorObject.h"
#include "editor/EdComponentWindows.h"
#include "platform/Platform.h"
#include "universe/Universe.h"
#include "universe/World.h"
#include "engine/Events.h"
#include "engine/Engine.h"


#include <glfw/glfw3.h>


namespace ed {

CaptionMenuBar::CaptionMenuBar(EditorObject_& parent)
	: editor(parent)
{
}

void CaptionMenuBar::MakeMainMenu()
{
	mainMenu.Clear();

	auto& sceneMenu = mainMenu.AddSubMenu("Scene");

	sceneMenu.AddEntry(U8(FA_FILE u8"  New"), [&]() { openPopupNewLevel.Set(); });
	sceneMenu.AddEntry(U8(FA_SAVE u8"  Save"), [&]() { editor.SaveLevel(); });
	sceneMenu.AddEntry(U8(FA_SAVE u8"  Save As"), [&]() { editor.SaveLevelAs(); });
	sceneMenu.AddEntry(U8(FA_SAVE u8"  Save All"), [&]() { editor.SaveAll(); });
	sceneMenu.AddEntry(U8(FA_FOLDER_OPEN u8"  Load"), [&]() { editor.OpenLoadDialog(); });
	sceneMenu.AddSeperator();
	sceneMenu.AddEntry(U8(FA_REDO_ALT u8"  Revert"), [&]() { Universe::ReloadMainWorld(); });
	sceneMenu.AddEntry(
		U8(FA_REDO_ALT u8"  Delete Local"), [&]() { openPopupDeleteLocal.Set(); }, {},
		[&]() { return fs::relative(Universe::MainWorld->srcPath) == "local.json"; });

	sceneMenu.AddSeperator();
	sceneMenu.AddEntry(U8(FA_DOOR_OPEN u8"  Exit"), []() { glfwSetWindowShouldClose(Platform::GetMainHandle(), 1); });


	auto& windowsMenu = mainMenu.AddSubMenu("Windows");
	for (auto& catEntry : editor.m_windowsComponent.m_categories) {
		windowsMenu.AddSubMenu(catEntry);
	}
	windowsMenu.AddSeperator();

	for (auto& winEntry : editor.m_windowsComponent.m_entries) {
		windowsMenu.AddOptionalCategory(
			winEntry.category, winEntry.name.c_str(), [&]() { editor.m_windowsComponent.ToggleUnique(winEntry.hash); },
			[&]() { return editor.m_windowsComponent.IsUniqueOpen(winEntry.hash); });
	}
}

namespace {
	void DrawPlayStopFpsWidget(EditorObject_& editor, ImVec2 scaledSize)
	{
		auto world = editor.m_currentWorld;
		const char* icon = world->IsStopped() ? U8(FA_PLAY) : U8(FA_STOP);

		fmt::basic_memory_buffer<char, 100> s;
		fmt::format_to(s, " {}  Raygen - {:.1f} FPS ###PlayStopCaptionTxt", icon, Engine.GetFPS());
		auto size = ImGui::CalcTextSize(s.data(), nullptr, true);
		float sizeW = std::round(size.x * 0.1f) / 0.1f; // Round some pixels to avoid resizing everyframe

		ImGui::SetCursorPosX((scaledSize.x / 2.f) - size.x / 2.f);
		if (ImEd::ButtonNoBorderText(s.data(), ImVec2(sizeW, size.y), true)) {
			if (world->IsPlaying()) {
				world->EndPlay();
			}
			else if (world->IsStopped()) {
				world->BeginPlay();
			}
		}
	}
} // namespace
void CaptionMenuBar::DrawBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 6.f)); // On edit update imextras.h c_MenuPaddingY
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 12.f));
	if (!ImGui::BeginMenuBar()) {
		ImGui::PopStyleVar(2);
		return;
	}
	ImGui::SetCursorPosX(
		ImGui::GetCursorPosX() - 1.f); // Offset to draw at left most pixel (allow clicking at 0,0 pixel when maximized)

	// Draw Menus
	auto initialCursorPos = ImGui::GetCursorPos();
	ImVec2 scaledSize = ImGui::GetCurrentWindow()->MenuBarRect().GetSize();
	mainMenu.DrawOptions(glm::vec2{ 1.f, 7.f }, glm::vec2{ 10.f, 7.f });

	// Additional save button after the last menu.
	if (ImEd::ButtonIcon(FA_SAVE)) {
		editor.SaveLevel();
	}

	// Draw FPS & play/stop widget in the middle:
	DrawPlayStopFpsWidget(editor, scaledSize);

	// Draw window buttons (minimize, resize exit)
	constexpr float c_menuItemWidth = 30.f;
	auto rightSideCursPos = initialCursorPos;
	rightSideCursPos.x = initialCursorPos.x + scaledSize.x - (3.f * (c_menuItemWidth + 20.f));
	ImGui::SetCursorPos(rightSideCursPos);

	if (ImGui::MenuItem(U8(u8"  " FA_WINDOW_MINIMIZE), nullptr, nullptr, true, c_menuItemWidth)) {
		glfwIconifyWindow(Platform::GetMainHandle());
	}

	auto middleText = editor.m_isMaximised ? U8(u8"  " FA_WINDOW_RESTORE) //
										   : U8(u8"  " FA_WINDOW_MAXIMIZE);

	if (ImGui::MenuItem(middleText, nullptr, nullptr, true, c_menuItemWidth)) {
		editor.m_isMaximised ? glfwRestoreWindow(Platform::GetMainHandle())
							 : glfwMaximizeWindow(Platform::GetMainHandle());
	}

	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EdColor::Red);
	if (ImGui::MenuItem(U8(u8"  " FA_WINDOW_CLOSE), nullptr, nullptr, true, c_menuItemWidth)) {
		glfwSetWindowShouldClose(Platform::GetMainHandle(), true);
	}
	ImGui::PopStyleColor();

	ImGui::SetCursorPos(initialCursorPos);
	if (ImGui::InvisibleButton("##MainMenuBarItemButton", scaledSize, ImGuiButtonFlags_PressedOnClick)) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			editor.m_isMaximised ? glfwRestoreWindow(Platform::GetMainHandle())
								 : glfwMaximizeWindow(Platform::GetMainHandle());
		}
		else {
			editor.PushCommand([]() { glfwDragWindow(Platform::GetMainHandle()); });
		}
	}

	ImGui::EndMenuBar();
	ImGui::PopStyleVar(2);
}

void CaptionMenuBar::Draw()
{
	DrawBar();

	ImEd::OkCancelModal("Delete Local", "Local scene will be lost. Are you sure?\n", openPopupDeleteLocal, []() {
		fs::remove("local.json");
		if (!fs::copy_file("engine-data/default.json", "local.json")) {
			LOG_ERROR("Failed to copy default world file to local.");
		}
		else {
			Universe::LoadMainWorld("local.json");
		}
	});

	ImEd::OkCancelModal(
		"New Level", "Unsaved changes will be lost.\nAre you sure?", openPopupNewLevel, [&]() { editor.NewLevel(); });
}
} // namespace ed
