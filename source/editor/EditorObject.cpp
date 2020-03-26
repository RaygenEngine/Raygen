#include "pch.h"
#include "Editor.h"

#include "editor/DataStrings.h"
#include "editor/EdUserSettings.h"
#include "editor/imgui/ImGuizmo.h"
#include "editor/misc/NativeFileBrowser.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "editor/windows/general/EdPropertyEditorWindow.h"
#include "editor/windows/WindowsRegistry.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/RootNode.h"
#include "universe/Universe.h"
#include "universe/WorldOperationsUtl.h"

#include <glfw/glfw3.h>


EditorObject::EditorObject()
{
	ImguiImpl::InitContext();

	m_nodeContextActions = std::make_unique<NodeContextActions>();

	Event::OnWorldNodeRemoved.Bind(this, [&](Node* node) {
		if (node == m_selectedNode) {
			if (node->GetParent()) {
				m_selectedNode = node->GetParent();
			}
			else {
				m_selectedNode = nullptr;
			}
		}
		if (node == m_editorCamera) {
			m_editorCamera = nullptr;
		}
	});

	Event::OnWorldLoaded.Bind(this, [&] { SpawnEditorCamera(); });

	MakeMainMenu();
}

Node* EditorObject::GetSelectedNode()
{
	auto editor = EditorObj;
	//	if (editor && Engine.IsEditorActive()) {
	return editor->m_selectedNode;
	//}
	return nullptr;
}

void EditorObject::MakeMainMenu()
{
	RegisterWindows(m_windowsComponent);


	m_menus.clear();
	auto sceneMenu = std::make_unique<ImMenu>("Scene");
	sceneMenu->AddEntry(U8(FA_SAVE u8"  Save As"), [&]() { m_sceneSave.OpenBrowser(); });
	sceneMenu->AddEntry(U8(FA_FOLDER_OPEN u8"  Load"), [&]() { OpenLoadDialog(); });
	sceneMenu->AddEntry(U8(FA_REDO_ALT u8"  Revert"), [&]() { ReloadScene(); });
	sceneMenu->AddSeperator();
	sceneMenu->AddEntry(U8(FA_DOOR_OPEN u8"  Exit"), []() { glfwSetWindowShouldClose(Platform::GetMainHandle(), 1); });
	m_menus.emplace_back(std::move(sceneMenu));


	auto windowsMenu = std::make_unique<ImMenu>("Windows");
	for (auto& winEntry : m_windowsComponent.m_entries) {
		windowsMenu->AddEntry(
			winEntry.name.c_str(), [&]() { m_windowsComponent.ToggleUnique(winEntry.hash); },
			[&]() { return m_windowsComponent.IsUniqueOpen(winEntry.hash); });
	}

	m_menus.emplace_back(std::move(windowsMenu));
}

EditorObject::~EditorObject()
{
	ImguiImpl::CleanupContext();
}


void EditorObject::Dockspace()
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

void EditorObject::UpdateViewportCoordsFromDockspace()
{
	auto centralNode = ImGui::DockBuilderGetCentralNode(m_dockspaceId);

	if (centralNode) {
		auto copy = g_ViewportCoordinates;
		auto rect = centralNode->Rect();


		g_ViewportCoordinates.position
			= { rect.Min.x - ImGui::GetWindowViewport()->Pos.x, rect.Min.y - ImGui::GetWindowViewport()->Pos.y };
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

void EditorObject::UpdateEditor()
{
	PROFILE_SCOPE(EditorObject);
	if (m_editorCamera) {
		m_editorCamera->UpdateFromEditor(Universe::GetMainWorld()->GetDeltaTime());
	}
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


	if (ImEd::Button(U8(FA_SAVE u8"  Save As"))) {
		m_sceneSave.OpenBrowser();
	}
	ImGui::SameLine();
	if (ImEd::Button(U8(FA_FOLDER_OPEN u8"  Load"))) {
		OpenLoadDialog();
	}

	std::string s = fmt::format("{:.1f} FPS", Engine.GetFPS());
	ImGui::Text(s.c_str());
	ImGui::End();


	ImguiImpl::EndFrame();

	for (auto& cmd : m_postDrawCommands) {
		cmd();
	}
	m_postDrawCommands.clear();

	ed::GetSettings().SaveIfDirty();
}

void EditorObject::OnFileDrop(std::vector<fs::path>&& files)
{
	m_windowsComponent.GetUniqueWindow<ed::AssetsWindow>()->ImportFiles(std::move(files));
}

void EditorObject::SpawnEditorCamera()
{
	auto world = Universe::GetMainWorld();
	m_editorCamera = NodeFactory::NewNode<EditorCameraNode>();
	m_editorCamera->SetName("Editor Camera");
	world->Z_RegisterNode(m_editorCamera, world->GetRoot());

	auto prevActive = world->GetActiveCamera();
	if (m_hasEditorCameraCachedMatrix) {
		m_editorCamera->SetNodeTransformWCS(m_editorCameraCachedMatrix);
	}
	else if (prevActive) {
		m_editorCamera->SetNodeTransformWCS(prevActive->GetNodeTransformWCS());
	}
	world->SetActiveCamera(m_editorCamera);
}

void EditorObject::OpenLoadDialog()
{
	if (auto file = ed::NativeFileBrowser::OpenFile({ "json" })) {
		LOG_REPORT("LOADING: {}", *file);
		m_sceneToLoad = *file;
	}
}

void EditorObject::LoadScene(const fs::path& scenefile)
{
	// TODO:
	LOG_ERROR("Reimplement Scene Loading.");
	return;

	m_updateWorld = false;
	m_selectedNode = nullptr;
}

void EditorObject::ReloadScene()
{
	// TODO:
	// auto path = AssetHandlerManager::GetPodUri(Universe::GetMainWorld()->GetLoadedFromHandle());
	// m_sceneToLoad = uri::ToSystemPath(path);
}

void EditorObject::OnDisableEditor()
{
	OnPlay();
}

void EditorObject::OnEnableEditor()
{
	OnStopPlay();
}

void EditorObject::OnPlay()
{
	if (m_editorCamera) {
		m_editorCameraCachedMatrix = m_editorCamera->GetNodeTransformWCS();
		m_hasEditorCameraCachedMatrix = true;
		Universe::GetMainWorld()->DeleteNode(m_editorCamera);
	}
	m_hasRestoreSave = false;
	if (m_autoRestoreWorld) {
		SceneSave::SaveAs(Universe::GetMainWorld(), "__scene.tmp");
		m_hasRestoreSave = true;
	}
}

void EditorObject::OnStopPlay()
{
	if (!m_updateWorld) {
		if (!m_editorCamera) {
			SpawnEditorCamera();
			m_editorCamera->SetNodeTransformWCS(m_editorCameraCachedMatrix);
		}
	}
	if (m_autoRestoreWorld && m_hasRestoreSave) {
		m_sceneToLoad = "__scene.tmp";
	}
}

void EditorObject::Run_MenuBar()
{
	if (ImEd::BeginMenuBar()) {
		for (auto& entry : m_menus) {
			entry->Draw(this);
		}
		ImEd::EndMenuBar();
	}
}

void EditorObject::HandleInput()
{
	auto& input = Engine.GetInput();
	if (input.IsDown(Key::LeftShift) && input.IsJustPressed(Key::F)) {
		PilotThis(m_selectedNode);
	}
	else if (input.IsDown(Key::F)) {
		if (m_selectedNode) {
			FocusNode(m_selectedNode);
		}
	}

	if (!input.IsDown(Key::Mouse_RightClick)) {
		using op = ed::ManipOperationMode::Operation;

		auto& manipMode = m_windowsComponent.GetUniqueWindow<ed::PropertyEditorWindow>()->m_manipMode;

		if (input.IsJustPressed(Key::W)) {
			manipMode.op = op::Translate;
		}
		else if (input.IsJustPressed(Key::E)) {
			manipMode.op = op::Rotate;
		}
		else if (input.IsJustPressed(Key::R)) {
			manipMode.op = op::Scale;
		}

		if (input.IsJustPressed(Key::Q)) {
			manipMode.mode = manipMode.mode == ed::ManipOperationMode::Space::Local
								 ? ed::ManipOperationMode::Space::World
								 : ed::ManipOperationMode::Space::Local;
		}
	}
}

void EditorObject::PushCommand(std::function<void()>&& func)
{
	EditorObj->m_postDrawCommands.emplace_back(func);
}

void EditorObject::PushPostFrameCommand(std::function<void()>&& func)
{
	EditorObj->m_postFrameCommands.emplace_back(func);
}


void EditorObject::PreBeginFrame()
{
	if (!m_sceneToLoad.empty()) {
		LoadScene(m_sceneToLoad);
		m_sceneToLoad = "";
	}

	for (auto& cmd : m_postFrameCommands) {
		cmd();
	}
	m_postFrameCommands.clear();
}

void EditorObject::SelectNode(Node* node)
{
	EditorObj->m_selectedNode = node;
}

void EditorObject::MoveSelectedUnder(Node* node)
{
	if (EditorObj->m_selectedNode != EditorObj->m_editorCamera) {
		worldop::MakeChildOf(node, EditorObj->m_selectedNode);
	}
}

void EditorObject::Duplicate(Node* node)
{
	PushCommand([node]() { Universe::GetMainWorld()->DeepDuplicateNode(node); });
}

void EditorObject::Delete(Node* node)
{
	PushCommand([node]() { Universe::GetMainWorld()->DeleteNode(node); });
}

void EditorObject::PilotThis(Node* node)
{
	auto camera = EditorObj->m_editorCamera;
	if (!camera) {
		LOG_WARN("Only possible to pilot nodes if there is an editor camera");
		return;
	}

	bool wasPiloting = EditorObj->IsCameraPiloting();

	if (camera->GetParent() == node || node == nullptr) {
		if (!wasPiloting) {
			return;
		}
		worldop::MakeChildOf(Universe::GetMainWorld()->GetRoot(), camera);
		camera->SetNodeTransformWCS(EditorObj->m_editorCameraPrePilotPos);
		return;
	}

	if (!wasPiloting) {
		EditorObj->m_editorCameraPrePilotPos = camera->GetNodeTransformWCS();
	}
	worldop::MakeChildOf(node, camera);
	camera->SetNodeTransformWCS(node->GetNodeTransformWCS());
}

void EditorObject::FocusNode(Node* node)
{
	if (!node) {
		return;
	}
	auto cam = EditorObj->m_editorCamera;
	if (!cam) {
		return;
	}
	auto trans = node->GetNodePositionWCS();
	cam->SetNodePositionWCS(trans);

	float dist = 1.f;
	if (node->IsA<GeometryNode>()) {
		auto geom = static_cast<GeometryNode*>(node);
		auto min = geom->GetAABB().min;
		auto max = geom->GetAABB().max;
		dist = glm::abs(min.x - max.x) + glm::abs(min.y - max.y);

		cam->SetNodePositionWCS(geom->GetAABB().GetCenter());
	}
	cam->AddNodePositionOffsetWCS(glm::vec3(-1.f, 0.25f, 0.f) * dist);
	cam->SetNodeLookAtWCS(node->GetNodePositionWCS());
}

void EditorObject::TeleportToCamera(Node* node)
{
	auto camera = Universe::GetMainWorld()->GetActiveCamera();
	if (camera) {
		auto newMat = math::transformMat(
			node->GetNodeScaleWCS(), camera->GetNodeOrientationWCS(), camera->GetNodePositionWCS());
		node->SetNodeTransformWCS(newMat);
	}
}
