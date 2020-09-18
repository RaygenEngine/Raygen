#include "pch.h"
#include "Editor.h"

#include "editor/DataStrings.h"
#include "editor/EdUserSettings.h"
#include "editor/imgui/ImGuizmo.h"
#include "editor/imgui/ImEd.h"
#include "editor/misc/NativeFileBrowser.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "editor/windows/general/EdPropertyEditorWindow.h"
#include "editor/windows/WindowsRegistry.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "universe/Universe.h"
#include "editor/windows/general/EdEcsOutlinerWindow.h"
#include "universe/EcsWorld.h"
#include "engine/Events.h"
#include "rendering/scene/Scene.h"
#include "rendering/Layer.h"
#include "rendering/Renderer.h"
#include "rendering/passes/RaytracingPass.h"


#include <imguicolortextedit/TextEditor.h>


#include <nlohmann/json.hpp>
#include <glfw/glfw3.h>

#include <fstream>

EditorObject_::EditorObject_()
{
	ImguiImpl::InitContext();
	MakeMainMenu();
	Event::OnWindowMaximize.Bind(this, [&](bool newIsMaximized) { m_isMaximised = newIsMaximized; });
}

void EditorObject_::MakeMainMenu()
{
	RegisterWindows(m_windowsComponent);

	m_mainMenu.Clear();


	auto& sceneMenu = m_mainMenu.AddSubMenu("Scene");

	sceneMenu.AddEntry(U8(FA_FILE u8"  New"), [&]() { NewLevel(); });
	sceneMenu.AddEntry(U8(FA_SAVE u8"  Save"), [&]() { SaveLevel(); });
	sceneMenu.AddEntry(U8(FA_SAVE u8"  Save As"), [&]() { SaveLevelAs(); });
	sceneMenu.AddEntry(U8(FA_SAVE u8"  Save All"), [&]() { SaveAll(); });
	sceneMenu.AddEntry(U8(FA_FOLDER_OPEN u8"  Load"), [&]() { OpenLoadDialog(); });
	sceneMenu.AddSeperator();
	sceneMenu.AddEntry(U8(FA_REDO_ALT u8"  Revert"), [&]() { ReloadScene(); });
	sceneMenu.AddEntry(
		U8(FA_REDO_ALT u8"  Delete Local"), [&]() { m_openPopupDeleteLocal = true; }, {},
		[&]() { return fs::relative(Universe::ecsWorld->srcPath) == "local.json"; });
	sceneMenu.AddSeperator();
	sceneMenu.AddEntry(U8(FA_DOOR_OPEN u8"  Exit"), []() { glfwSetWindowShouldClose(Platform::GetMainHandle(), 1); });


	auto& windowsMenu = m_mainMenu.AddSubMenu("Windows");
	for (auto& catEntry : m_windowsComponent.m_categories) {
		windowsMenu.AddSubMenu(catEntry);
	}
	windowsMenu.AddSeperator();

	for (auto& winEntry : m_windowsComponent.m_entries) {
		windowsMenu.AddOptionalCategory(
			winEntry.category, winEntry.name.c_str(), [&]() { m_windowsComponent.ToggleUnique(winEntry.hash); },
			[&]() { return m_windowsComponent.IsUniqueOpen(winEntry.hash); });
	}
}

EditorObject_::~EditorObject_()
{
	ImguiImpl::CleanupContext();
}


void EditorObject_::Dockspace()
{
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags
		= ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen) {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
						| ImGuiWindowFlags_NoMove;
		window_flags
			|= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
	}

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	bool dummy{};
	ImGui::Begin("DockSpace", &dummy, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen) {
		ImGui::PopStyleVar(2);
	}


	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	m_dockspaceId = ImGui::GetID("MainDockspace");
	ImGui::DockSpace(m_dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);
	ImGuizmo::SetDrawlist();
	Run_MenuBar();


	ImGui::End();
}

void EditorObject_::UpdateViewportCoordsFromDockspace()
{
	auto centralNode = ImGui::DockBuilderGetCentralNode(m_dockspaceId);

	if (centralNode) {
		auto copy = g_ViewportCoordinates;
		auto rect = centralNode->Rect();

		auto rectMin = rect.Min;


		g_ViewportCoordinates.position
			= { rect.Min.x - ImGui::GetMainViewport()->Pos.x, rect.Min.y - ImGui::GetMainViewport()->Pos.y };
		g_ViewportCoordinates.size = { rect.GetWidth(), rect.GetHeight() };
		if (copy != g_ViewportCoordinates) {
			Event::OnViewportUpdated.Broadcast();
			edCamera.ResizeViewport(g_ViewportCoordinates.size);
		}
	}

	auto& coord = g_ViewportCoordinates;

	ImGuizmo::SetRect(ImGui::GetWindowViewport()->Pos.x + coord.position.x,
		ImGui::GetWindowViewport()->Pos.y + coord.position.y, static_cast<float>(coord.size.x),
		static_cast<float>(coord.size.y));
}

void EditorObject_::UpdateEditor()
{
	PROFILE_SCOPE(Editor);

	m_currentWorld = Universe::ecsWorld;

	for (auto& cmd : m_deferredCommands) {
		cmd();
	}
	m_deferredCommands.clear();

	// if (m_editorCamera) {
	// m_editorCamera->UpdateFromEditor(Universe::GetMainWorld()->GetDeltaTime());
	//}

	edCamera.Update(1.f / std::max(Engine.GetFPS(), 1.f));
	edCamera.EnqueueUpdateCmds(vl::Layer->mainScene);
	HandleInput();

	ImguiImpl::NewFrame();
	Dockspace();
	UpdateViewportCoordsFromDockspace();

	m_windowsComponent.Draw();


	// Attempt to predict the viewport size for the first run, might be a bit off.
	ImGui::SetNextWindowSize(ImVec2(450, 1042), ImGuiCond_FirstUseEver);
	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoScrollbar);


	if (ImGui::Checkbox("Update World", &m_updateWorld)) {
		if (m_updateWorld) {
			OnPlay();
		}
		else {
			OnStopPlay();
		}
	}
	ImEd::HelpTooltip(help_UpdateWorld);

	if (!(m_updateWorld && !m_hasRestoreSave)) {
		ImEd::HSpace(4.f);
		ImGui::SameLine();
		ImGui::Checkbox("Restore update state", &m_autoRestoreWorld);
		ImEd::HelpTooltip(help_RestoreWorld);
	}


	if (ImEd::Button(ETXT(FA_SAVE, "Save"))) {
		SaveLevel();
	}
	ImGui::SameLine();
	if (ImEd::Button(ETXT(FA_FOLDER_OPEN, "Load"))) {
		OpenLoadDialog();
	}

	std::string s = fmt::format("{:.1f} FPS : Rt Index: {}", Engine.GetFPS(), vl::Renderer->m_raytracingPass.m_rtFrame);
	ImGui::Text(s.c_str());

	ImGui::End();


	ImguiImpl::EndFrame();

	auto& g = *ImGui::GetCurrentContext();
	if (g.HoveredWindow == NULL && ImGui::GetTopMostPopupModal() == NULL && g.NavWindow != NULL
		&& g.IO.MouseClicked[1]) {
		ImGui::FocusWindow(nullptr);
		ImGui::ClearActiveID();
	}

	while (m_postDrawCommands.ConsumingRegion()) {
		for (auto& cmd : m_postDrawCommands.vec) {
			cmd();
		}
	}


	ed::GetSettings().SaveIfDirty();
}

void EditorObject_::OnFileDrop(std::vector<fs::path>&& files)
{
	m_windowsComponent.GetUniqueWindow<ed::AssetsWindow>()->ImportFiles(std::move(files));
}

void EditorObject_::SpawnEditorCamera()
{
	//
	// auto world = Universe::GetMainWorld();
	// m_editorCamera = NodeFactory::NewNode<EditorCameraNode>();
	// m_editorCamera->SetName("Editor Camera");
	// world->Z_RegisterNode(m_editorCamera, world->GetRoot());

	// auto prevActive = world->GetActiveCamera();
	// if (m_hasEditorCameraCachedMatrix) {
	//	m_editorCamera->SetNodeTransformWCS(m_editorCameraCachedMatrix);
	//}
	// else if (prevActive) {
	//	m_editorCamera->SetNodeTransformWCS(prevActive->GetNodeTransformWCS());
	//}
	// world->SetActiveCamera(m_editorCamera);
}

void EditorObject_::OpenLoadDialog()
{
	if (auto file = ed::NativeFileBrowser::OpenFile({ "json" })) {
		m_updateWorld = false;
		// m_selectedNode = nullptr;
		ed::EcsOutlinerWindow::selected = {};

		Universe::ECS_LoadMainWorld(*file);
	}
}

void EditorObject_::ReloadScene()
{
	Universe::ECS_LoadMainWorld(Universe::ecsWorld->srcPath);
}

void EditorObject_::OnDisableEditor()
{
	OnPlay();
}

void EditorObject_::OnEnableEditor()
{
	OnStopPlay();
}

void EditorObject_::OnPlay()
{
	// TODO: ECS
}

void EditorObject_::OnStopPlay()
{
	// WIP: ECS
}

namespace {
void MenuBar(ed::Menu& m_mainMenu) {}
} // namespace

void EditorObject_::TopMostMenuBarDraw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 7.f)); // On edit update imextras.h c_MenuPaddingY
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 7.f));
	if (!ImGui::BeginMenuBar()) {
		ImGui::PopStyleVar(2);
		return;
	}
	auto initialCursorPos = ImGui::GetCursorPos();
	ImVec2 scaledSize = ImGui::GetCurrentWindow()->MenuBarRect().GetSize();
	m_mainMenu.DrawOptions();

	constexpr float c_menuItemWidth = 30.f;
	auto rightSideCursPos = initialCursorPos;
	rightSideCursPos.x = initialCursorPos.x + scaledSize.x - (3.f * (c_menuItemWidth + 20.f));
	rightSideCursPos.x -= 2.f;
	ImGui::SetCursorPos(rightSideCursPos);


	if (ImGui::MenuItem(U8(u8"  " FA_WINDOW_MINIMIZE), nullptr, nullptr, true, c_menuItemWidth)) {
		glfwIconifyWindow(Platform::GetMainHandle());
	}

	auto middleText = m_isMaximised ? U8(u8"  " FA_WINDOW_RESTORE) : U8(u8"  " FA_WINDOW_MAXIMIZE);
	if (ImGui::MenuItem(middleText, nullptr, nullptr, true, c_menuItemWidth)) {
		m_isMaximised ? glfwRestoreWindow(Platform::GetMainHandle()) : glfwMaximizeWindow(Platform::GetMainHandle());
	}

	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EdColor::Red);
	if (ImGui::MenuItem(U8(u8"  " FA_WINDOW_CLOSE), nullptr, nullptr, true, c_menuItemWidth)) {
		glfwSetWindowShouldClose(Platform::GetMainHandle(), true);
	}
	ImGui::PopStyleColor();

	ImGui::SetCursorPos(initialCursorPos);
	if (ImGui::InvisibleButton("##MainMenuBarItemButton", scaledSize, ImGuiButtonFlags_PressedOnClick)) {
		glfwDragWindow(Platform::GetMainHandle());
		LOG_REPORT("Clicked");
	}

	ImGui::EndMenuBar();
	ImGui::PopStyleVar(2);
}

void EditorObject_::Run_MenuBar()
{
	TopMostMenuBarDraw();
	/*
	if (ImEd::BeginMenuBar()) {
		m_mainMenu.DrawOptions();
		ImEd::EndMenuBar();
	}
	*/

	if (m_openPopupDeleteLocal) {
		ImGui::OpenPopup("Delete Local");
		m_openPopupDeleteLocal = false;
	}

	if (ImGui::BeginPopupModal("Delete Local", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Local scene will be lost. Are you sure?\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 40))) {
			fs::remove("local.json");
			if (!fs::copy_file("engine-data/default.json", "local.json")) {
				LOG_ERROR("Failed to copy default world file to local.");
			}
			else {
				Universe::ECS_LoadMainWorld("local.json");
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 40))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void EditorObject_::HandleInput()
{
	if (!Input.IsDown(Key::Mouse_RightClick)) {
		using op = ed::ManipOperationMode::Operation;

		auto& manipMode = m_windowsComponent.GetUniqueWindow<ed::PropertyEditorWindow>()->m_manipMode;

		if (Input.IsJustPressed(Key::W)) {
			manipMode.op = op::Translate;
		}
		else if (Input.IsJustPressed(Key::E)) {
			manipMode.op = op::Rotate;
		}
		else if (Input.IsJustPressed(Key::R)) {
			manipMode.op = op::Scale;
		}

		if (Input.IsJustPressed(Key::Q)) {
			manipMode.mode = manipMode.mode == ed::ManipOperationMode::Space::Local
								 ? ed::ManipOperationMode::Space::World
								 : ed::ManipOperationMode::Space::Local;
		}
	}

	if (Input.IsJustPressed(Key::Delete)) {
		if (auto ent = Editor::GetSelection(); ent) {
			ent.Destroy();
		}
	}
}

void EditorObject_::PushCommand(std::function<void()>&& func)
{
	EditorObject->m_postDrawCommands.Emplace(func);
}

void EditorObject_::PushDeferredCommand(std::function<void()>&& func)
{
	EditorObject->m_deferredCommands.push_back(std::move(func));
}

void EditorObject_::SaveLevel()
{
	if (m_currentWorld && !m_currentWorld->srcPath.empty()) {
		m_currentWorld->SaveToDisk();
	}
	else if (m_currentWorld) {
		SaveLevelAs();
	}
}

void EditorObject_::SaveLevelAs()
{
	if (m_currentWorld) {
		if (auto f = ed::NativeFileBrowser::SaveFile({ "json" }); f) {
			if (!f->has_extension()) {
				f->replace_extension(".json");
			}
			m_currentWorld->SaveToDisk(*f, true);
		}
	}
}

void EditorObject_::SaveAll()
{
	SaveLevel();
	AssetHandlerManager::SaveAll();
}

void EditorObject_::NewLevel()
{
	Universe::ECS_LoadMainWorld("");
}

// WIP: ECS
// void EditorObject_::PilotThis(Node* node)
//{
//	auto camera = EditorObject->m_editorCamera;
//	if (!camera) {
//		LOG_WARN("Only possible to pilot nodes if there is an editor camera");
//		return;
//	}
//
//	bool wasPiloting = EditorObject->IsCameraPiloting();
//
//	if (camera->GetParent() == node || node == nullptr) {
//		if (!wasPiloting) {
//			return;
//		}
//		worldop::MakeChildOf(Universe::GetMainWorld()->GetRoot(), camera);
//		camera->SetNodeTransformWCS(EditorObject->m_editorCameraPrePilotPos);
//		return;
//	}
//
//	if (!wasPiloting) {
//		EditorObject->m_editorCameraPrePilotPos = camera->GetNodeTransformWCS();
//	}
//	worldop::MakeChildOf(node, camera);
//	camera->SetNodeTransformWCS(node->GetNodeTransformWCS());
//}
//
// void EditorObject_::FocusNode(Node* node)
//{
//	if (!node) {
//		return;
//	}
//	auto cam = EditorObject->m_editorCamera;
//	if (!cam) {
//		return;
//	}
//	auto trans = node->GetNodePositionWCS();
//	cam->SetNodePositionWCS(trans);
//
//	float dist = 1.f;
//
//	// if (node->IsA<GeometryNode>()) {
//	//	auto geom = static_cast<GeometryNode*>(node);
//	//	auto min = geom->GetAABB().min;
//	//	auto max = geom->GetAABB().max;
//	//	dist = glm::abs(min.x - max.x) + glm::abs(min.y - max.y);
//
//	//	cam->SetNodePositionWCS(geom->GetAABB().GetCenter());
//	//}
//	cam->AddNodePositionOffsetWCS(glm::vec3(-1.f, 0.25f, 0.f) * dist);
//	cam->SetNodeLookAtWCS(node->GetNodePositionWCS());
//}
//
// void EditorObject_::TeleportToCamera(Node* node)
//{
//	auto camera = Universe::GetMainWorld()->GetActiveCamera();
//	if (camera) {
//		auto newMat = math::transformMat(
//			node->GetNodeScaleWCS(), camera->GetNodeOrientationWCS(), camera->GetNodePositionWCS());
//		node->SetNodeTransformWCS(newMat);
//	}
//}
