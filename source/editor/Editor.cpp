#include "pch/pch.h"

#include "editor/Editor.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/NodeContextActions.h"
#include "editor/PropertyEditor.h"

#include "asset/AssetManager.h"
#include "asset/PodIncludes.h"
#include "platform/windows/Win32Window.h"
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
			m_selectedNode = nullptr;
		}
	});


	m_loadFileBrowser.SetTitle("Load Scene");

	MakeMainMenu();
}

struct ImRendererMenu : public Editor::ImMenu {
	ImRendererMenu()
		: Editor::ImMenu("Renderer")
	{
	}

	void DrawOptions(Editor* editor) override
	{
		if (ImGui::MenuItem("Restart")) {
			Editor::PushPostFrameCommand(
				[=]() { Engine::Get().SwitchRenderer(Engine::Get().GetActiveRendererIndex()); });
		}

		if (ImGui::BeginMenu("Switch")) {
			int32 index = 0;
			int32 current = Engine::Get().GetActiveRendererIndex();

			for (auto& r : Engine::Get().GetRendererList()) {
				if (ImGui::MenuItem(r.c_str(), nullptr, index == current)) {
					Editor::PushPostFrameCommand([=]() { Engine::Get().SwitchRenderer(index); });
				}
				++index;
			}
			ImGui::EndMenu();
		}
	}
};

void Editor::MakeMainMenu()
{
	auto sceneMenu = std::make_unique<ImMenu>("Scene");
	sceneMenu->AddEntry("Save", [&]() { m_sceneSave.OpenBrowser(); });
	sceneMenu->AddEntry("Load", [&]() { m_loadFileBrowser.Open(); });
	sceneMenu->AddSeperator();
	sceneMenu->AddEntry("Exit", []() { Engine::GetMainWindow()->Destroy(); });
	m_menus.emplace_back(std::move(sceneMenu));

	auto renderersMenu = std::make_unique<ImRendererMenu>();
	m_menus.emplace_back(std::move(renderersMenu));

	auto debugMenu = std::make_unique<ImMenu>("Debug");
	debugMenu->AddEntry("Open ImGui Demo", [&]() { m_showImguiDemo = true; });
	debugMenu->AddEntry("Open Log", [&]() { m_showLogWindow = true; });
	m_menus.emplace_back(std::move(debugMenu));

	auto aboutMenu = std::make_unique<ImMenu>("About");
	aboutMenu->AddEntry("Help", [&]() { m_showHelpWindow = true; });
	aboutMenu->AddEntry("About", [&]() { m_showAboutWindow = true; });
	m_menus.emplace_back(std::move(aboutMenu));
}


Editor::~Editor()
{
	ImguiImpl::CleanupContext();
}

void Editor::UpdateEditor()
{
	HandleInput();

	ImguiImpl::NewFrame();

	if (m_showImguiDemo) {
		ImGui::ShowDemoWindow();
	}

	if (m_showAboutWindow) {
		Run_AboutWindow();
	}

	if (m_showHelpWindow) {
		Run_HelpWindow();
	}

	if (m_showLogWindow) {
		Run_LogWindow();
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	// Attempt to predict the viewport size for the first run, might be a bit off.
	ImGui::SetNextWindowSize(ImVec2(450, 1042), ImGuiCond_FirstUseEver);
	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);

	Run_MenuBar();

	ImGui::Checkbox("Update World", &m_updateWorld);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
	if (ImGui::Button("Save")) {
		m_sceneSave.OpenBrowser();
	}
	ImGui::SameLine();

	if (ImGui::Button("Load")) {
		m_loadFileBrowser.SetTitle("Load World");
		m_loadFileBrowser.Open();
	}
	ImGui::PopStyleVar();


	m_sceneSave.Draw();
	m_loadFileBrowser.Display();

	if (m_loadFileBrowser.HasSelected()) {
		m_sceneToLoad = m_loadFileBrowser.GetSelected();
		m_loadFileBrowser.ClearSelected();
	}

	if (ImGui::BeginChild("EditorScrollable", ImVec2(0, -15.f))) {
		if (ImGui::CollapsingHeader("Outliner", ImGuiTreeNodeFlags_DefaultOpen)) {
			Outliner();
		}

		if (m_selectedNode) {
			if (ImGui::CollapsingHeader(
					refl::GetClass(m_selectedNode).GetNameStr().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				m_propertyEditor->Inject(m_selectedNode);
			}
		}

		if (ImGui::CollapsingHeader("Assets")) {
			Run_AssetView();
		}
	}

	ImGui::EndChild();

	std::string s = fmt::format("{:.1f} FPS | {}", Engine::GetFPS(), Engine::GetStatusLine());
	ImGui::Text(s.c_str());


	ImGui::End();

	if (m_showGltfWindow) {
		m_showGltfWindow = m_assetWindow->Draw();
	}


	ImguiImpl::EndFrame();

	for (auto& cmd : m_postDrawCommands) {
		cmd();
	}
	m_postDrawCommands.clear();
}

void Editor::Outliner()
{
	ImGui::BeginChild(
		"Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Spacing();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 6.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 6.f));

	RecurseNodes(Engine::GetWorld()->GetRoot(), [&](Node* node, int32 depth) {
		auto str = std::string(depth * 6, ' ') + sceneconv::FilterNodeClassName(node->GetClass().GetName()) + "> "
				   + node->m_name;
		ImGui::PushID(node->GetUID());
		if (ImGui::Selectable(str.c_str(), node == m_selectedNode)) {
			m_selectedNode = node;
		}
		Run_ContextPopup(node);
		Run_OutlinerDropTarget(node);
		ImGui::PopID();
	});

	ImGui::PopStyleVar(2);
	ImGui::EndChild();
}

void Editor::LoadScene(const fs::path& scenefile)
{
	Engine::Get().CreateWorldFromFile("/" + fs::relative(scenefile).string());
	Engine::Get().SwitchRenderer(Engine::Get().GetActiveRendererIndex());

	m_selectedNode = nullptr;
	Event::OnWindowResize.Broadcast(Engine::GetMainWindow()->GetWidth(), Engine::GetMainWindow()->GetHeight());
}

void Editor::Run_ContextPopup(Node* node)
{
	if (ImGui::BeginPopupContextItem("Outliner Context")) {
		for (auto& action : m_nodeContextActions->GetActions(node)) {
			if (!action.IsSplitter()) {
				if (ImGui::MenuItem(action.name)) {
					action.function(node);
				}
			}
			else {
				ImGui::Separator();
			}
		}

		if (ImGui::BeginMenu("New Node")) {
			Run_NewNodeMenu(node);
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
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
	auto reloadAssetLambda = [](std::unique_ptr<PodEntry>& assetEntry) {
		auto l = [&assetEntry](auto tptr) {
			using PodType = std::remove_pointer_t<decltype(tptr)>;
			PodHandle<PodType> p;
			p.podId = assetEntry->uid;
			AssetManager::Reload(p);
		};

		podtools::VisitPodType(assetEntry->type, l);
	};


	ImGui::Indent(10.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
	bool UnloadAll = ImGui::Button("Unload All");
	ImGui::SameLine();
	bool ReloadUnloaded = ImGui::Button("Reload Unloaded");
	ImGui::SameLine();
	bool ReloadAll = ImGui::Button("Reload All");
	ImGui::SameLine();
	if (ImGui::Button("Search for GLTFs")) {
		m_showGltfWindow = true;
	}

	// Static meh..
	static ImGuiTextFilter filter;
	filter.Draw("Filter Assets", ImGui::GetFontSize() * 16);

	ImGui::PopStyleVar();


	std::string text;
	for (auto& assetEntry : Engine::GetAssetManager()->m_pods) {
		if (!filter.PassFilter(assetEntry->path.c_str()) && !filter.PassFilter(assetEntry->name.c_str())) {
			continue;
		}

		ImGui::PushID(static_cast<int32>(assetEntry->uid));
		bool disabled = !(assetEntry->ptr);

		if (disabled) {
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.6f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.7f, 0.6f));
			if (ImGui::Button("Reload") || ReloadUnloaded) {
				reloadAssetLambda(assetEntry);
			}
			ImGui::PopStyleColor(3);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
		}
		else {
			if (ImGui::Button("Unload") || UnloadAll) {
				AssetManager::Unload(BasePodHandle{ assetEntry->uid });
			}
		}

		if (ReloadAll) {
			reloadAssetLambda(assetEntry);
		}

		ImGui::SameLine();
		ImGui::Text(assetEntry->path.c_str());
		if (disabled) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		Run_MaybeAssetTooltip(assetEntry.get());


		ImGui::PopID();
	}
}

void Editor::Run_MaybeAssetTooltip(PodEntry* entry)
{
	if (ImGui::IsItemHovered()) {
		std::string text = fmt::format("Path:\t{}\nName:\t{}\nType:\t{}\n Ptr:\t{}\n UID:\t{}", entry->path,
			entry->name, entry->type.name(), entry->ptr, entry->uid);

		if (!entry->ptr) {
			ImUtil::TextTooltipUtil(text);
			return;
		}

		text += "\n";
		text += "\n";
		text += "\n";

		text += refltools::PropertiesToText(entry->ptr.get());

		ImUtil::TextTooltipUtil(text);
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
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 3.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 6.f));

	if (ImGui::BeginMenuBar()) {
		for (auto& entry : m_menus) {
			entry->Draw(this);
		}
		ImGui::EndMenuBar();
	}
	ImGui::PopStyleVar(2);
}

void Editor::Run_AboutWindow()
{
	constexpr float version = 1.f;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
	ImGui::SetNextWindowSize(ImVec2(550.f, 300.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
	if (ImGui::Begin("About", &m_showAboutWindow)) {
		auto str = fmt::format(R"(
Rayxen Engine:

Version: {}
Rayxen is a graphics/game engine focused on renderer extensibility.

Authors:
John Moschos - Founder, Programmer
Harry Katagis - Programmer
)",
			version);

		ImGui::Text(str.c_str());
		ImGui::Text("");
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void Editor::Run_HelpWindow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
	ImGui::SetNextWindowSize(ImVec2(550.f, 300.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Help", &m_showHelpWindow)) {
		auto str = fmt::format(R"(
Help:

TODO: INSERT HELP HERE!
)");
		ImGui::Text(str.c_str());
		ImGui::Text("");
	}
	ImGui::End();
	ImGui::PopStyleVar();
}


void Editor::HandleInput()
{
	if (Engine::GetInput()->IsKeyRepeat(XVirtualKey::O)) {
		LOG_ERROR(
			"Some error happened OMG! Some error happened OMG!Some error happened OMG!Some error happened OMG!Some "
			"error happened OMG!Some error happened OMG!Some error happened OMG!Some error happened OMG!");
		LOG_ERROR("Some error happened OMG!");
		LOG_WARN("LMAO");
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
	// WIP: decide what flag this sets
}

void Editor::MoveChildOut(Node* node)
{
	if (node->GetParent()->IsRoot()) {
		return;
	}
	auto lateCmd = [node]() {
		auto worldMatrix = node->GetWorldMatrix();
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
		node->SetWorldMatrix(worldMatrix);
		node->SetDirty(Node::DF::Hierarchy);
		node->GetParent()->SetDirty(Node::DF::Children);
	};

	PushCommand(lateCmd);
}


void Editor::MoveSelectedUnder(Node* node)
{
	MakeChildOf(node, Engine::GetEditor()->m_selectedNode);
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
		auto worldMatrix = node->GetWorldMatrix();
		node->GetParent()->SetDirty(Node::DF::Children); // That parent is losing a child.

		auto& children = node->GetParent()->m_children;
		std::vector<NodeUniquePtr>::iterator thisIt
			= std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

		NodeUniquePtr src = std::move(*thisIt);

		children.erase(thisIt);

		newParent->m_children.emplace_back(std::move(src));

		node->m_parent = newParent;
		node->SetWorldMatrix(worldMatrix);
		node->SetDirty(Node::DF::Hierarchy);
		node->GetParent()->SetDirty(Node::DF::Children);
	};

	PushCommand(lateCmd);
}


void Editor::LookThroughThis(Node* node)
{
	auto camera = Engine::GetWorld()->GetActiveCamera();
	if (camera) {
		camera->SetWorldMatrix(node->GetWorldMatrix());
	}
}

void Editor::TeleportToCamera(Node* node)
{
	auto camera = Engine::GetWorld()->GetActiveCamera();
	if (camera) {
		node->SetWorldMatrix(camera->GetWorldMatrix());
	}
}

void Editor::MakeActiveCamera(Node* node)
{
	if (node->IsA<CameraNode>()) {
		Engine::GetWorld()->m_activeCamera = static_cast<CameraNode*>(node);
	}
}

namespace logwindow {
struct ExampleAppLog {
	ImGuiTextBuffer Buf;
	ImGuiTextFilter Filter;
	ImVector<int> LineOffsets; // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a
							   // random access on lines
	bool AutoScroll;           // Keep scrolling if already at the bottom

	ExampleAppLog()
	{
		AutoScroll = true;
		Clear();
	}

	void Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
	}

	void Draw(const char* title, bool* p_open = NULL)
	{
		if (!ImGui::Begin(title, p_open)) {
			ImGui::End();
			return;
		}

		// Options menu
		if (ImGui::BeginPopup("Options")) {
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (clear)
			Clear();
		if (copy)
			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (Filter.IsActive()) {
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of
			// search/filter. especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end
					= (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
				if (Filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else {
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to
			// skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
			// within the visible area. If you have tens of thousands of items and their processing cost is
			// non-negligible, coarse clipping them on your side is recommended. Using ImGuiListClipper requires A)
			// random access into your data, and B) items all being the  same height, both of which we can handle since
			// we an array pointing to the beginning of each line of text. When using the filter (in the block of code
			// above) we don't have random access into the data to display anymore, which is why we don't use the
			// clipper. Storing or skimming through the search result would make it possible (and would be recommended
			// if you want to search through tens of thousands of entries)
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step()) {
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end
						= (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
		ImGui::End();
	}
};

} // namespace logwindow

void Editor::Run_LogWindow()
{
	static logwindow::ExampleAppLog log;
	auto& ss = utl::Log::s_editorLogStream;


	std::string line;
	while (std::getline(ss, line)) {
		log.AddLog(line.c_str());
		log.AddLog("\n");
	}
	ss.clear();

	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Rayxen Log", &m_showLogWindow);
	ImGui::End();

	log.Draw("Rayxen Log", &m_showLogWindow);
}
