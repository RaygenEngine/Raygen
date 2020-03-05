
#include "pch.h"

#include "editor/Editor.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/NodeContextActions.h"
#include "editor/PropertyEditor.h"

#include "asset/AssetManager.h"
#include "asset/PodIncludes.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"
#include "system/Engine.h"
#include "system/EngineEvents.h"
#include "world/nodes/camera/CameraNode.h"

#include "world/nodes/Node.h"
#include "world/nodes/RootNode.h"
#include "world/World.h"
#include "system/Input.h"
#include "world/NodeFactory.h"

#include "editor/imgui/ImguiUtil.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "reflection/ReflEnum.h"
#include "editor/DataStrings.h"
#include "editor/imgui/ImEd.h"

#include "system/console/Console.h"
#include "editor/windows/WindowsRegistry.h"
#include "system/profiler/ProfileScope.h"

#include "editor/EdUserSettings.h"
#include "system/console/ConsoleVariable.h"

#include "editor/misc/NativeFileBrowser.h"
#include "editor/utl/EdAssetUtils.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "editor/imgui/ImGuizmo.h"
#include "renderer/VulkanLayer.h"

#include <glfw/glfw3.h>

#include <iostream>
#include <set>

Editor::Editor()
{
	ImguiImpl::InitContext();

	m_assetWindow = std::make_unique<AssetWindow>();
	m_nodeContextActions = std::make_unique<NodeContextActions>();
	m_propertyEditor = std::make_unique<PropertyEditor>();

	m_onNodeRemoved.Bind([&](Node* node) {
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

	m_onWorldLoaded.Bind([&]() { SpawnEditorCamera(); });

	MakeMainMenu();
}

struct ImRendererMenu : public Editor::ImMenu {
	ImRendererMenu()
		: Editor::ImMenu("Renderer")
	{
	}

	void DrawOptions(Editor* editor) override
	{
		if (ImGui::BeginMenu("BBox Debugging")) {
			auto& bboxEnum = ReflEnum::GetMeta<EditorBBoxDrawing>();
			auto bboxEnumTie = bboxEnum.TieEnum(editor->m_bboxDrawing);

			for (auto& [value, str] : bboxEnum.GetValues()) {
				std::string s;
				s = str;
				if (ImGui::MenuItem(s.c_str(), nullptr, bboxEnumTie.GetValue() == value)) {
					bboxEnumTie.SetValue(value);
				}
			}
			ImGui::EndMenu();
		}
	}
};

Node* Editor::GetSelectedNode()
{
	auto editor = Engine::GetEditor();
	if (editor && Engine::IsEditorActive()) {
		return editor->m_selectedNode;
	}
	return nullptr;
}

EditorBBoxDrawing Editor::GetBBoxDrawing()
{
	auto editor = Engine::GetEditor();
	if (editor && Engine::IsEditorActive()) {
		return editor->m_bboxDrawing;
	}
	return EditorBBoxDrawing::None;
}

void Editor::MakeMainMenu()
{
	RegisterWindows(m_windowsComponent);


	m_menus.clear();
	auto sceneMenu = std::make_unique<ImMenu>("Scene");
	sceneMenu->AddEntry(U8(FA_SAVE u8"  Save As"), [&]() { m_sceneSave.OpenBrowser(); });
	sceneMenu->AddEntry(U8(FA_FOLDER_OPEN u8"  Load"), [&]() { OpenLoadDialog(); });
	sceneMenu->AddEntry(U8(FA_REDO_ALT u8"  Revert"), [&]() { ReloadScene(); });
	sceneMenu->AddSeperator();
	sceneMenu->AddEntry(U8(FA_DOOR_OPEN u8"  Exit"), []() { glfwSetWindowShouldClose(Engine::GetMainWindow(), 1); });
	m_menus.emplace_back(std::move(sceneMenu));

	auto renderersMenu = std::make_unique<ImRendererMenu>();
	m_menus.emplace_back(std::move(renderersMenu));

	auto windowsMenu = std::make_unique<ImMenu>("Windows");
	for (auto& winEntry : m_windowsComponent.m_entries) {
		windowsMenu->AddEntry(
			winEntry.name.c_str(), [&]() { m_windowsComponent.ToggleUnique(winEntry.hash); },
			[&]() { return m_windowsComponent.IsUniqueOpen(winEntry.hash); });
	}

	m_menus.emplace_back(std::move(windowsMenu));
}

Editor::~Editor()
{
	ImguiImpl::CleanupContext();
}


void Editor::Dockspace()
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

void Editor::UpdateViewportCoordsFromDockspace()
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

namespace {
void DrawTextureDebugger()
{
	ImGui::Begin("Image Debugger.");

	auto descrSet = *VulkanLayer::quadDescriptorSet;
	if (!descrSet) {
		ImGui::Text("Null handle");
		ImGui::End();
		return;
	}

	ImGui::Image(descrSet, ImVec2(512, 512));
	ImGui::End();
}
} // namespace

void Editor::UpdateEditor()
{
	PROFILE_SCOPE(Editor);
	if (m_editorCamera) {
		m_editorCamera->UpdateFromEditor(Engine::GetWorld()->GetDeltaTime());
	}
	HandleInput();

	ImguiImpl::NewFrame();
	Dockspace();

	m_windowsComponent.Draw();


	DrawTextureDebugger();

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
	HelpTooltip(help_UpdateWorld);

	if (!(m_updateWorld && !m_hasRestoreSave)) {
		ImEd::HSpace(4.f);
		ImGui::SameLine();
		ImGui::Checkbox("Restore update state", &m_autoRestoreWorld);
		HelpTooltip(help_RestoreWorld);
	}


	if (ImEd::Button(U8(FA_SAVE u8"  Save As"))) {
		m_sceneSave.OpenBrowser();
	}
	ImGui::SameLine();
	if (ImEd::Button(U8(FA_FOLDER_OPEN u8"  Load"))) {
		OpenLoadDialog();
	}


	auto linesAtBottom = Engine::GetStatusLine().empty() ? 1 : 2;

	if (ImGui::BeginChild("EditorScrollable", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * linesAtBottom))) {
		auto open = ImGui::CollapsingHeader("Outliner", ImGuiTreeNodeFlags_DefaultOpen);
		CollapsingHeaderTooltip(help_Outliner);
		if (open) {
			Outliner();
		}

		if (m_selectedNode) {
			open = ImGui::CollapsingHeader(
				refl::GetClass(m_selectedNode).GetNameStr().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
			CollapsingHeaderTooltip(help_PropertyEditor);
			if (open) {
				m_propertyEditor->Inject(m_selectedNode);
			}
		}

		open = ImGui::CollapsingHeader("Assets");
		CollapsingHeaderTooltip(help_AssetView);
		if (open) {
			Run_AssetView();
		}
	}

	ImGui::EndChild();

	auto& eng = Engine::Get();


	auto drawReporter = Engine::GetDrawReporter();
	std::string s = fmt::format(
		"Draws: {:n} | Tris: {:n} | {:.1f} FPS", drawReporter->draws, drawReporter->tris, Engine::GetFPS());

	ImGui::Text(s.c_str());

	if (!Engine::GetStatusLine().empty()) {
		ImGui::Text(Engine::GetStatusLine().c_str());
	}


	ImGui::End();

	if (m_showGltfWindow) {
		m_showGltfWindow = m_assetWindow->Draw();
	}

	UpdateViewportCoordsFromDockspace();
	ImguiImpl::EndFrame();

	for (auto& cmd : m_postDrawCommands) {
		cmd();
	}
	m_postDrawCommands.clear();

	ed::GetSettings().SaveIfDirty();
}

void Editor::OnFileDrop(std::vector<fs::path>&& files)
{
	m_windowsComponent.GetUniqueWindow<ed::AssetsWindow>()->ImportFiles(std::move(files));
}


void Editor::SpawnEditorCamera()
{
	auto world = Engine::GetWorld();
	m_editorCamera = NodeFactory::NewNode<EditorCameraNode>();
	m_editorCamera->SetName("Editor Camera");
	world->RegisterNode(m_editorCamera, world->GetRoot());

	auto prevActive = world->GetActiveCamera();
	if (m_hasEditorCameraCachedMatrix) {
		m_editorCamera->SetNodeTransformWCS(m_editorCameraCachedMatrix);
	}
	else if (prevActive) {
		m_editorCamera->SetNodeTransformWCS(prevActive->GetNodeTransformWCS());
	}
	world->SetActiveCamera(m_editorCamera);
}

void Editor::Outliner()
{
	PROFILE_SCOPE(Editor);

	ImGui::BeginChild(
		"Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Spacing();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 6.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 6.f));

	bool foundOpen = false;
	RecurseNodes(Engine::GetWorld()->GetRoot(), [&](Node* node, int32 depth) {
		auto str = std::string(depth * 6, ' ') + U8(node->GetClass().GetIcon()) + "  "
				   + sceneconv::FilterNodeClassName(node->GetClass().GetName()) + "> " + node->m_name;
		ImGui::PushID(node->GetUID());

		if (node == m_editorCamera) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.58f, 0.58f, 0.95f));
			// ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.5f, 0.0f, 0.0f, 0.95f));
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.28f, 0.01f, 0.10f, 0.95f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.3f, 0.02f, 0.09f, 0.95f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.36f, 0.04f, 0.11f, 0.95f));
		}

		if (ImGui::Selectable(str.c_str(), node == m_selectedNode)) {
			m_selectedNode = node;
		}

		if (node != m_editorCamera) {
			Run_ContextPopup(node);
			Run_OutlinerDropTarget(node);
		}
		else {
			ImGui::PopStyleColor(4);

			if (ImGui::BeginPopupContextItem("OutlinerElemContext")) {
				if (!m_editorCamera->GetParent()->IsRoot()) {
					if (ImGui::MenuItem("Stop piloting")) {
						Editor::MakeChildOf(Engine::GetWorld()->GetRoot(), m_editorCamera);
						m_editorCamera->ResetRotation();
					}
				}
				if (ImGui::MenuItem("Set As Active")) {
					Engine::GetWorld()->SetActiveCamera(m_editorCamera);
				}
				ImGui::EndPopup();
			}
			else {
				Editor::CollapsingHeaderTooltip(help_EditorCamera);
			}
		}
		if (ImGui::IsPopupOpen("OutlinerElemContext")) {
			foundOpen = true;
		}
		ImGui::PopID();
	});

	ImGui::PopStyleVar(2);
	ImGui::EndChild();

	if (auto entry = ImEd::AcceptTypedPodDrop<ModelPod>()) {
		auto cmd = [&, entry]() {
			auto newNode = NodeFactory::NewNode<GeometryNode>();

			newNode->SetName(entry->GetNameStr());
			newNode->SetModel(entry->GetHandleAs<ModelPod>());
			Engine::GetWorld()->RegisterNode(newNode, Engine::GetWorld()->GetRoot());
			if (!IsCameraPiloting()) {
				PushPostFrameCommand([newNode]() { FocusNode(newNode); });
			}
		};

		PushCommand(cmd);
	}

	/*
	if (ImGui::BeginDragDropTarget()) {
		std::string payloadTag = "POD_UID_" + std::to_string(refl::GetId<ModelPod>().hash());
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag.c_str())) {
			assert(payload->DataSize == sizeof(size_t));
			size_t uid = *reinterpret_cast<size_t*>(payload->Data);

			auto* podEntry = AssetHandlerManager::GetEntry(BasePodHandle{ uid });

			auto cmd = [&, uid, podEntry]() {
				auto newNode = NodeFactory::NewNode<GeometryNode>();

				newNode->SetName(podEntry->name);
				newNode->SetModel(PodHandle<ModelPod>{ uid });
				Engine::GetWorld()->RegisterNode(newNode, Engine::GetWorld()->GetRoot());
				if (!IsCameraPiloting()) {
					PushPostFrameCommand([newNode]() { FocusNode(newNode); });
				}
			};

			PushCommand(cmd);
		}
		ImGui::EndDragDropTarget();
	}
	*/
	if (!foundOpen) {
		ImGui::PushID(989);
		if (ImGui::BeginPopupContextItem("RightclickOutliner Context")) {
			if (ImGui::BeginMenu("New Node")) {
				Run_NewNodeMenu(Engine::GetWorld()->GetRoot());
				ImGui::EndMenu();
			}
			if (IsCameraPiloting() && ImGui::MenuItem("Stop piloting")) {
				Editor::PilotThis(nullptr);
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}

void Editor::OpenLoadDialog()
{
	if (auto file = ed::NativeFileBrowser::OpenFile({ "json" })) {
		LOG_REPORT("LOADING: {}", *file);
		m_sceneToLoad = *file;
	}
}

void Editor::LoadScene(const fs::path& scenefile)
{
	// TODO:
	LOG_ERROR("Reimplement Scene Loading.");
	return;

	m_updateWorld = false;
	m_selectedNode = nullptr;

	Engine::Get().CreateWorldFromFile(fs::relative(scenefile).string());
	// Engine::Get().SwitchRenderer();

	int32 width;
	int32 height;

	glfwGetWindowSize(Engine::GetMainWindow(), &width, &height);
	Event::OnWindowResize.Broadcast(width, height);
}

void Editor::ReloadScene()
{
	auto path = AssetHandlerManager::GetPodUri(Engine::GetWorld()->GetLoadedFromHandle());
	m_sceneToLoad = uri::ToSystemPath(path);
}

void Editor::OnDisableEditor()
{
	OnPlay();
}

void Editor::OnEnableEditor()
{
	OnStopPlay();
}

void Editor::OnPlay()
{
	if (m_editorCamera) {
		m_editorCameraCachedMatrix = m_editorCamera->GetNodeTransformWCS();
		m_hasEditorCameraCachedMatrix = true;
		Engine::GetWorld()->DeleteNode(m_editorCamera);
	}
	m_hasRestoreSave = false;
	if (m_autoRestoreWorld) {
		SceneSave::SaveAs(Engine::GetWorld(), "__scene.tmp");
		m_hasRestoreSave = true;
	}
}

void Editor::OnStopPlay()
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

bool Editor::Run_ContextPopup(Node* node)
{
	if (ImGui::BeginPopupContextItem("OutlinerElemContext")) {
		for (auto& action : m_nodeContextActions->GetActions(node, true)) {
			if (!action.IsSplitter()) {
				if (ImGui::MenuItem(action.name)) {
					action.function(node);
				}
			}
			else {
				ImGui::Separator();
			}
		}
		ImGui::Separator();

		if (ImGui::BeginMenu("Add Child")) {
			Run_NewNodeMenu(node);
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
		return true;
	}
	return false;
}

void Editor::Run_NewNodeMenu(Node* underNode)
{
	auto factory = Engine::GetWorld()->m_nodeFactory;


	for (auto& entry : factory->m_nodeEntries) {
		if (ImGui::MenuItem(entry.first.c_str())) {

			auto cmd = [underNode, &entry]() {
				auto newNode = entry.second.newInstance();

				newNode->SetName(entry.first + "_new");
				Engine::GetWorld()->RegisterNode(newNode, underNode);

				DirtyFlagset temp;
				temp.set();

				newNode->SetDirtyMultiple(temp);
			};

			PushCommand(cmd);
		}
	}
}

void Editor::Run_AssetView()
{
	ImGui::Indent(10.f);
	if (ImEd::Button("Search for GLTFs")) {
		m_showGltfWindow = true;
	}
	Editor::HelpTooltipInline(help_AssetSearchGltf);

	// Static meh..
	static ImGuiTextFilter filter;
	filter.Draw("Filter Assets", ImGui::GetFontSize() * 16);
	HelpTooltip(help_AssetFilter);

	std::string text;
	for (auto& assetEntry : AssetHandlerManager::Z_GetPods()) {
		if (!filter.PassFilter(assetEntry->path.c_str()) && !filter.PassFilter(assetEntry->name.c_str())) {
			continue;
		}

		ImGui::PushID(static_cast<int32>(assetEntry->uid));

		ImGui::Text(assetEntry->path.c_str());
		ed::asset::MaybeHoverTooltip(assetEntry.get());

		ImGui::PopID();
	}
}

void Editor::Run_OutlinerDropTarget(Node* node)
{
	// Drag Source
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
		ImGui::SetDragDropPayload("WORLD_REORDER", &node, sizeof(Node**));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WORLD_REORDER")) {
			CLOG_ABORT(payload->DataSize != sizeof(Node**), "Incorrect drop operation.");
			Node** dropSource = reinterpret_cast<Node**>(payload->Data);
			Editor::MakeChildOf(node, *dropSource);
		}
		ImGui::EndDragDropTarget();
	}
}

void Editor::Run_MenuBar()
{
	if (ImEd::BeginMenuBar()) {
		for (auto& entry : m_menus) {
			entry->Draw(this);
		}
		ImEd::EndMenuBar();
	}
}

void Editor::HandleInput()
{
	auto& input = Engine::GetInput();
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

		if (input.IsJustPressed(Key::W)) {
			m_propertyEditor->m_manipMode.op = op::Translate;
		}
		else if (input.IsJustPressed(Key::E)) {
			m_propertyEditor->m_manipMode.op = op::Rotate;
		}
		else if (input.IsJustPressed(Key::R)) {
			m_propertyEditor->m_manipMode.op = op::Scale;
		}

		if (input.IsJustPressed(Key::Q)) {
			m_propertyEditor->m_manipMode.mode
				= m_propertyEditor->m_manipMode.mode == ed::ManipOperationMode::Space::Local
					  ? ed::ManipOperationMode::Space::World
					  : ed::ManipOperationMode::Space::Local;
		}
	}
}
void Editor::PushCommand(std::function<void()>&& func)
{
	Engine::GetEditor()->m_postDrawCommands.emplace_back(func);
}

void Editor::PushPostFrameCommand(std::function<void()>&& func)
{
	Engine::GetEditor()->m_postFrameCommands.emplace_back(func);
}

void Editor::HelpTooltip(const char* tooltip)
{
	if (!Editor::s_showHelpTooltips) {
		return;
	}
	ImEd::HSpace(1.f);
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 0.95f));
	ImGui::TextUnformatted("\xc2\xb0"); // help symbol: aka U+00b0
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
void Editor::HelpTooltipInline(const char* tooltip)
{
	if (!Editor::s_showHelpTooltips) {
		return;
	}
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void Editor::CollapsingHeaderTooltip(const char* tooltip)
{
	if (!Editor::s_showHelpTooltips) {
		return;
	}
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 12.f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.6f, 0.95f));
	ImGui::TextUnformatted("?");
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}


void Editor::PreBeginFrame()
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

void Editor::Duplicate(Node* node)
{
	PushCommand([node]() { Engine::GetWorld()->DeepDuplicateNode(node); });
}

void Editor::Delete(Node* node)
{
	PushCommand([node]() { Engine::GetWorld()->DeleteNode(node); });
}

void Editor::MoveChildUp(Node* node)
{
	auto& children = node->GetParent()->m_children;

	auto thisIt = std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	// Should be impossible unless world ptrs are corrupted
	CLOG_ABORT(thisIt == end(children), "Attempting to move child not in its parent container.");

	if (thisIt != begin(children)) {
		std::iter_swap(thisIt, thisIt - 1);
	}
	node->SetDirty(Node::DF::Children);
}

void Editor::MoveChildDown(Node* node)
{
	auto& children = node->GetParent()->m_children;
	auto thisIt = std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	// Should be impossible unless world ptrs are corrupted
	CLOG_ABORT(thisIt == end(children), "Attempting to move child not in its parent container.");

	if (thisIt + 1 != end(children)) {
		std::iter_swap(thisIt, thisIt + 1);
	}
	node->SetDirty(Node::DF::Children);
}

void Editor::MoveChildOut(Node* node)
{
	if (node->GetParent()->IsRoot()) {
		return;
	}
	auto lateCmd = [node]() {
		auto worldMatrix = node->GetNodeTransformWCS();
		node->GetParent()->SetDirty(Node::DF::Children);

		auto& children = node->GetParent()->m_children;
		std::vector<NodeUniquePtr>::iterator thisIt
			= std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

		NodeUniquePtr src = std::move(*thisIt);

		children.erase(thisIt);

		Node* insertBefore = node->GetParent();
		auto& newChildren = insertBefore->GetParent()->m_children;

		auto insertAt = std::find_if(
			begin(newChildren), end(newChildren), [&insertBefore](auto& elem) { return insertBefore == elem.get(); });

		newChildren.emplace(insertAt, std::move(src));

		node->m_parent = insertBefore->GetParent();
		node->SetNodeTransformWCS(worldMatrix);
		node->SetDirty(Node::DF::Hierarchy);
		node->GetParent()->SetDirty(Node::DF::Children);
	};

	PushCommand(lateCmd);
}


void Editor::MoveSelectedUnder(Node* node)
{
	if (Engine::GetEditor()->m_selectedNode != Engine::GetEditor()->m_editorCamera) {
		MakeChildOf(node, Engine::GetEditor()->m_selectedNode);
	}
}


void Editor::MakeChildOf(Node* newParent, Node* node)
{
	if (!node || !newParent || node == newParent) {
		return;
	}

	// We cannot move a parent to a child. Start from node and iterate to root. If we find "selectedNode" we cannot
	// move.
	Node* pathNext = newParent->GetParent();
	while (pathNext) {
		if (pathNext == node) {
			LOG_INFO("Cannot move '{}' under '{}' because the second is a child of the first.", node->GetName(),
				newParent->GetName());
			return;
		}
		pathNext = pathNext->GetParent();
	}

	auto lateCmd = [newParent, node]() {
		auto worldMatrix = node->GetNodeTransformWCS();
		node->GetParent()->SetDirty(Node::DF::Children); // That parent is losing a child.

		auto& children = node->GetParent()->m_children;
		std::vector<NodeUniquePtr>::iterator thisIt
			= std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

		NodeUniquePtr src = std::move(*thisIt);

		children.erase(thisIt);

		newParent->m_children.emplace_back(std::move(src));

		node->m_parent = newParent;
		node->SetNodeTransformWCS(worldMatrix);
		node->SetDirty(Node::DF::Hierarchy);
		node->GetParent()->SetDirty(Node::DF::Children);
	};

	PushCommand(lateCmd);
}

void Editor::PilotThis(Node* node)
{
	auto camera = Engine::GetEditor()->m_editorCamera;
	if (!camera) {
		LOG_WARN("Only possible to pilot nodes if there is an editor camera");
		return;
	}

	bool wasPiloting = Engine::GetEditor()->IsCameraPiloting();

	if (camera->GetParent() == node || node == nullptr) {
		if (!wasPiloting) {
			return;
		}
		Editor::MakeChildOf(Engine::GetWorld()->GetRoot(), camera);
		camera->SetNodeTransformWCS(Engine::GetEditor()->m_editorCameraPrePilotPos);
		return;
	}

	if (!wasPiloting) {
		Engine::GetEditor()->m_editorCameraPrePilotPos = camera->GetNodeTransformWCS();
	}
	Editor::MakeChildOf(node, camera);
	camera->SetNodeTransformWCS(node->GetNodeTransformWCS());
}

void Editor::FocusNode(Node* node)
{
	if (!node) {
		return;
	}
	auto cam = Engine::GetEditor()->m_editorCamera;
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
//		auto newMat
//= math::TransformMatrixFromTOS(camera->GetTranslation(), camera->GetOrientation(), node->GetScale());
// node->SetMatrix(newMat);
void Editor::TeleportToCamera(Node* node)
{
	auto camera = Engine::GetWorld()->GetActiveCamera();
	if (camera) {
		auto newMat = math::transformMat(
			node->GetNodeScaleWCS(), camera->GetNodeOrientationWCS(), camera->GetNodePositionWCS());
		node->SetNodeTransformWCS(newMat);
	}
}

void Editor::MakeActiveCamera(Node* node)
{
	if (node->IsA<CameraNode>()) {
		Engine::GetWorld()->SetActiveCamera(NodeCast<CameraNode>(node));
	}
}
