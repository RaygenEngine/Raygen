#include "EditorObject.h"

#include "assets/pods/Mesh.h"
#include "editor/EdUserSettings.h"
#include "editor/Editor.h"
#include "editor/misc/NativeFileBrowser.h"
#include "editor/windows/WindowsRegistry.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "editor/windows/general/EdOutlinerWindow.h"
#include "editor/windows/general/EdPropertyEditorWindow.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/Layer.h"
#include "universe/ComponentsDb.h"
#include "universe/Universe.h"
#include "universe/components/StaticMeshComponent.h"

#include <glfw/glfw3.h>

EditorObject_::EditorObject_()
	: m_captionBar(*this)
{
	ImguiImpl::InitContext();
	RegisterWindows(m_windowsComponent);
	m_captionBar.MakeMainMenu();

	Event::OnWindowMaximize.Bind(this, [&](bool newIsMaximized) { m_isMaximised = newIsMaximized; });
	Event::OnWindowResize.Bind(this, [&](int32 width, int32 height) {
		if (!m_drawUi) {
			g_ViewportCoordinates.position = { 0, 0 };
			g_ViewportCoordinates.size = { width, height };
			Event::OnViewportUpdated.Broadcast();
		}
	});
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


	m_captionBar.Draw();

	ImGui::End();
}

void EditorObject_::HandleClickSelection(bool wasCtrl)
{
	m_currentWorld->physics.Create(*m_currentWorld);
	glm::vec3 cameraFwd = edCamera.transform.front();
	auto mouseUv = Input.GetMouseViewportUV();

	auto remappedMouse = mouseUv * 2.f - 1.f;


	auto dir = glm::normalize(glm::vec3(
		glm::inverse(edCamera.proj * edCamera.view) * glm::vec4(remappedMouse.x, -remappedMouse.y, 1.0f, 1.0f)));

	if (wasCtrl) {
		auto result = Universe::MainWorld->physics.RayCastChitGeom(edCamera.transform.position, dir * (edCamera.far));

		ed::OutlinerWindow::selected = result.entity;

		m_windowsComponent.CloseAsset(lastClickAssetSelected);
		if (result.entity && result.geomGroupIndex >= 0) {
			// Detect and open material in this geometry group for editing
			auto matHandle = result.entity.Get<CStaticMesh>().mesh.Lock()->materials[result.geomGroupIndex];
			m_windowsComponent.OpenAsset(matHandle);
			lastClickAssetSelected = matHandle;
		}
	}
	else {
		auto result
			= Universe::MainWorld->physics.RayCastChitSelection(edCamera.transform.position, dir * (edCamera.far));
		ed::OutlinerWindow::selected = result.entity;
	}
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

	m_currentWorld = Universe::MainWorld;

	for (auto& cmd : m_deferredCommands) {
		cmd();
	}
	m_deferredCommands.clear();

	edCamera.Update(1.f / std::max(Engine.GetFPS(), 1.f)); // TODO: fix 1 / fps
	edCamera.EnqueueUpdateCmds(vl::Layer->mainScene);

	HandleInput();


	ImguiImpl::NewFrame();
	if (m_drawUi) {
		Dockspace();
		UpdateViewportCoordsFromDockspace();

		m_windowsComponent.Draw();
	}
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

void EditorObject_::OpenLoadDialog()
{
	if (auto file = ed::NativeFileBrowser::OpenFile({ "json" })) {
		ed::OutlinerWindow::selected = {};
		Universe::LoadMainWorld(*file);
	}
}

void EditorObject_::BeforePlayWorld(World& world)
{
	if (&world == m_currentWorld) {
		m_lastPlayedWorld.clear();
		ComponentsDb::RegistryToJson(world, m_lastPlayedWorld);
	}
}

void EditorObject_::AfterStopWorld(World& world)
{
	if (&world == m_currentWorld) {
		world.ResetWorld();
		ComponentsDb::JsonToRegistry(m_lastPlayedWorld, world);
		vl::Layer->ResetMainScene();
	}
}

void EditorObject_::HandleInput()
{
	// TODO: remove statics when done with input system
	static timer::Timer clickTimer;
	static bool wasCtrlDown{ false };

	if (Input.IsJustPressed(Key::Mouse_LeftClick)) {
		// TODO: move timers to input
		// TODO: implement MouseHasDragged() returns true if mouse did drag during this click (query from just released)
		clickTimer.Start();
		wasCtrlDown = Input.IsDown(Key::Ctrl);
	}

	if (Input.IsJustReleased(Key::Mouse_LeftClick) && Input.IsMouseInViewport()) {
		if (clickTimer.Get<ch::milliseconds>() < 250) {
			HandleClickSelection(wasCtrlDown);
		}
	}


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

	if (Input.IsJustPressed(Key::G)) {
		m_drawUi = !m_drawUi;
		if (!m_drawUi) {
			int32 w, h;
			glfwGetWindowSize(Platform::GetMainHandle(), &w, &h);
			g_ViewportCoordinates.position = { 0, 0 };
			g_ViewportCoordinates.size = { w, h };
			Event::OnViewportUpdated.Broadcast();
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
		LOG_REPORT("Level saved at: {}", m_currentWorld->srcPath);
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
	AssetRegistry::SaveAll();
}

void EditorObject_::NewLevel()
{
	Universe::LoadMainWorld("");
}
