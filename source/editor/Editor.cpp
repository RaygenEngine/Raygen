#include "pch/pch.h"

#include "editor/Editor.h"
#include "editor/imgui/ImguiImpl.h"
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

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui_ext/imfilebrowser.h>
#include <imgui/imgui_internal.h>
#include <iostream>
#include <set>


#define TEXT_TOOLTIP(...)                                                                                              \
	if (ImGui::IsItemHovered()) {                                                                                      \
		TextTooltipUtil(fmt::format(__VA_ARGS__));                                                                     \
	}

namespace {
void TextTooltipUtil(const std::string& Tooltip)
{
	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 45.0f);
	ImGui::TextUnformatted(Tooltip.c_str());
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}
} // namespace


namespace {
inline float* FromVec3(glm::vec3& vec3)
{
	return reinterpret_cast<float*>(&vec3);
}
inline float* FromVec4(glm::vec4& vec4)
{
	return reinterpret_cast<float*>(&vec4);
}


using namespace PropertyFlags;

struct ReflectionToImguiVisitor {
	int32 depth{ 0 };
	std::string path{};

	std::set<std::string> objNames;

	std::string nameBuf;
	const char* name;

	Node* node;

	DirtyFlagset dirtyFlags;

	void GenerateUniqueName(const Property& p)
	{
		std::string buf = p.GetNameStr() + "##" + path;
		auto r = objNames.insert(buf);
		int32 index = 0;
		while (!r.second) {
			r = objNames.insert(buf + std::to_string(index));
			index++;
		}
		name = r.first->c_str();
	}

	void Begin(const ReflClass& r) { path += "|" + r.GetNameStr(); }

	void End(const ReflClass& r) { path.erase(path.end() - (r.GetNameStr().size() + 1), path.end()); }

	void PreProperty(const Property& p) { GenerateUniqueName(p); }

	template<typename T>
	void operator()(T& t, const Property& p)
	{
		if (Inner(t, p)) {
			if (p.GetDirtyFlagIndex() >= 0) {
				dirtyFlags.set(p.GetDirtyFlagIndex());
			}
			dirtyFlags.set(Node::DF::Properties);
		}
	}

	bool Inner(int32& t, const Property& p) { return ImGui::DragInt(name, &t, 0.1f); }

	bool Inner(bool& t, const Property& p)
	{
		if (p.HasFlags(NoEdit)) {
			bool t1 = t;
			ImGui::Checkbox(name, &t1);
			return false;
		}
		return ImGui::Checkbox(name, &t);
	}

	bool Inner(float& t, const Property& p) { return ImGui::DragFloat(name, &t, 0.01f); }

	bool Inner(glm::vec3& t, const Property& p)
	{
		if (p.HasFlags(PropertyFlags::Color)) {
			return ImGui::ColorEdit3(name, FromVec3(t), ImGuiColorEditFlags_DisplayHSV);
		}

		return ImGui::DragFloat3(name, FromVec3(t), 0.01f);
	}

	bool Inner(glm::vec4& t, const Property& p)
	{
		if (p.HasFlags(PropertyFlags::Color)) {
			return ImGui::ColorEdit4(name, FromVec4(t), ImGuiColorEditFlags_DisplayHSV);
		}

		return ImGui::DragFloat4(name, FromVec4(t), 0.01f);
	}

	bool Inner(std::string& ref, const Property& p)
	{
		if (p.HasFlags(PropertyFlags::Multiline)) {
			return ImGui::InputTextMultiline(name, &ref, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
		}

		return ImGui::InputText(name, &ref);
	}

	template<typename PodType>
	void PodDropTarget(PodHandle<PodType>& pod)
	{
		//		std::string payloadTag = "POD_UID_" + std::to_string(pod->type.hash());

		if (ImGui::BeginDragDropTarget()) {
			std::string payloadTag = "POD_UID_" + std::to_string(ctti::type_id<PodType>().hash());
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag.c_str())) {
				assert(payload->DataSize == sizeof(size_t));
				size_t uid = *reinterpret_cast<size_t*>(payload->Data);
				pod.podId = uid;
				dirtyFlags.set(Node::DF::Properties);
			}
			ImGui::EndDragDropTarget();
		}
	}

	template<typename PodType>
	bool Inner(PodHandle<PodType>& pod, const Property& p)
	{
		if (!pod.HasBeenAssigned()) {
			std::string s = "Unitialised handle: " + p.GetNameStr();
			ImGui::Text(s.c_str());
			return false;
		}

		auto str = AssetManager::GetEntry(pod)->name;
		bool open = ImGui::CollapsingHeader(name);
		PodDropTarget(pod);
		TEXT_TOOLTIP("{}", AssetManager::GetPodUri(pod));
		if (open) {
			GenerateUniqueName(p);
			ImGui::InputText(name, &str, ImGuiInputTextFlags_ReadOnly);
			PodDropTarget(pod);


			depth++;
			ImGui::Indent();

			refltools::CallVisitorOnEveryProperty(const_cast<PodType*>(pod.operator->()), *this);

			ImGui::Unindent();
			depth--;
		}
		return false;
	}

	template<typename T>
	bool Inner(T& t, const Property& p)
	{
		std::string s = "unhandled property: " + p.GetNameStr();
		ImGui::Text(s.c_str());
		return false;
	}

	template<typename T>
	bool Inner(std::vector<PodHandle<T>>& t, const Property& p)
	{
		if (ImGui::CollapsingHeader(name)) {
			ImGui::Indent();
			int32 index = 0;
			for (auto& handle : t) {
				++index;
				std::string sname = "|" + p.GetNameStr() + std::to_string(index);
				size_t len = sname.size();
				path += sname;

				GenerateUniqueName(p);
				std::string finalName = AssetManager::GetEntry(handle)->name + "##" + name;

				bool r = ImGui::CollapsingHeader(finalName.c_str());
				PodDropTarget(handle);
				TEXT_TOOLTIP("{}", AssetManager::GetPodUri(handle));
				if (r) {
					ImGui::Indent();
					refltools::CallVisitorOnEveryProperty(const_cast<T*>(handle.operator->()), *this);
					ImGui::Unindent();
				}

				path.erase(path.end() - (len), path.end());
			}
			ImGui::Unindent();
		}
		return false;
	}

	// Enum
	bool Inner(MetaEnumInst& t, const Property& p)
	{
		auto enumMeta = t.GetEnum();

		// int32 currentItem = ;
		std::string str;
		str = t.GetValueStr();
		int32 currentValue = t.GetValue();

		if (ImGui::BeginCombo(
				name, str.c_str())) // The second parameter is the label previewed before opening the combo.
		{
			for (auto& [enumStr, value] : enumMeta.GetStringsToValues()) {
				bool selected = (currentValue == value);
				if (ImGui::Selectable(enumStr.c_str(), &selected)) {
					t.SetValue(value);
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return false;
	}
};


} // namespace


Editor::Editor()
{
	ImguiImpl::InitContext();
	m_assetWindow.Init();
}

Editor::~Editor()
{
	ImguiImpl::CleanupContext();
}

#include "system/Input.h"

void Editor::UpdateEditor()
{
	HandleInput();

	ImguiImpl::NewFrame();

	static ImGui::FileBrowser lfb = ImGui::FileBrowser(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);

	ImGui::ShowDemoWindow();

	static bool open = true;

	ImGui::Begin("Editor", &open);
	ImGui::Checkbox("Update World", &m_updateWorld);

	if (ImGui::Button("Save")) {
		m_sceneSave.OpenBrowser();
	}

	m_sceneSave.Draw();
	ImGui::SameLine();

	if (ImGui::Button("Load")) {
		lfb.SetTitle("Load World");
		lfb.Open();
	}

	lfb.Display();

	if (lfb.HasSelected()) {
		m_sceneToLoad = lfb.GetSelected();
		lfb.ClearSelected();
	}

	if (ImGui::CollapsingHeader("Outliner", ImGuiTreeNodeFlags_DefaultOpen)) {
		Outliner();
	}

	if (m_selectedNode) {
		if (ImGui::CollapsingHeader(
				refl::GetClass(m_selectedNode).GetNameStr().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			PropertyEditor(m_selectedNode);
		}
	}

	if (ImGui::CollapsingHeader("Assets")) {

		auto reloadAssetLambda = [](std::unique_ptr<PodEntry>& assetEntry) {
			auto l = [&assetEntry](auto tptr) {
				using PodType = std::remove_pointer_t<decltype(tptr)>;
				PodHandle<PodType> p;
				p.podId = assetEntry->uid;
				AssetManager::Reload(p);
			};

			podtools::VisitPodType(assetEntry->type, l);
		};


		ImGui::Indent();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
		bool UnloadAll = ImGui::Button("Unload All");
		ImGui::SameLine();
		bool ReloadUnloaded = ImGui::Button("Reload Unloaded");
		ImGui::SameLine();
		bool ReloadAll = ImGui::Button("Reload All");
		ImGui::PopStyleVar();

		std::string text;
		for (auto& assetEntry : Engine::GetAssetManager()->m_pods) {
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

			TEXT_TOOLTIP("Path: {}\n Ptr: {}\nType: {}\n UID: {}", assetEntry->path, assetEntry->ptr,
				assetEntry->type.name(), assetEntry->uid);
			ImGui::PopID();
		}
	}

	ImGui::End();


	m_assetWindow.Draw();


	ImguiImpl::EndFrame();
}

void Editor::Outliner()
{
	ImGui::BeginChild(
		"Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);


	RecurseNodes(Engine::GetWorld()->GetRoot(), [&](Node* node, int32 depth) {
		auto str = std::string(depth * 4, ' ') + node->m_type + "> " + node->m_name;
		ImGui::PushID(node->GetUID());
		if (ImGui::Selectable(str.c_str(), node == m_selectedNode)) {
			m_selectedNode = node;
		}
		if (ImGui::BeginPopupContextItem("Outliner Context")) {
			if (ImGui::Selectable("Teleport To Camera")) {
				auto camera = Engine::GetWorld()->GetActiveCamera();
				if (camera) {
					node->SetWorldMatrix(camera->GetWorldMatrix());
				}
			}
			if (ImGui::Selectable("Pilot")) {
				ImGui::OpenPopup("Clone Name");
			}

			ImGui::EndPopup();
		}
		ImGui::PopID();
	});
	ImGui::EndChild();
}

void Editor::PropertyEditor(Node* node)
{
	ImGui::BeginChild("Properties", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::InputText("Name", &node->m_name);

	glm::vec3 eulerPyr = glm::degrees(glm::eulerAngles(node->m_localOrientation));

	bool dirtyMatrix = false;

	if (ImGui::DragFloat3("Position", FromVec3(node->m_localTranslation), 0.01f)) {
		dirtyMatrix = true;
	}
	if (ImGui::DragFloat3("Rotation", FromVec3(eulerPyr), 0.1f)) {
		auto axes = node->GetLocalPYR();
		axes = eulerPyr - axes;

		node->m_localOrientation = node->m_localOrientation * glm::quat(glm::radians(axes));
		dirtyMatrix = true;
	}
	if (ImGui::DragFloat3("Scale", FromVec3(node->m_localScale), 0.01f)) {
		dirtyMatrix = true;
	}


	if (ImGui::Button("Duplicate")) {
		Node* newnode = Engine::GetWorld()->DeepDuplicateNode(node);
		m_selectedNode = newnode;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete")) {
		Engine::GetWorld()->DeleteNode(node);
		m_selectedNode = nullptr;
		ImGui::EndChild();
		return;
	}


	auto camera = dynamic_cast<CameraNode*>(node);
	if (camera) {
		ImGui::SameLine();
		if (ImGui::Button("Make Active Camera")) {
			Engine::GetWorld()->m_activeCamera = camera;
		}
	}

	ImGui::Separator();
	ReflectionToImguiVisitor visitor;
	visitor.node = node;
	visitor.dirtyFlags = node->GetDirtyFlagset();
	refltools::CallVisitorOnEveryProperty(node, visitor);

	node->m_dirty = visitor.dirtyFlags;

	ImGui::EndChild();


	if (dirtyMatrix) {
		node->SetLocalMatrix(
			math::TransformMatrixFromTOS(node->m_localScale, node->m_localOrientation, node->m_localTranslation));
	}
}

void Editor::LoadScene(const fs::path& scenefile)
{

	Engine::Get().CreateWorldFromFile("/" + fs::relative(scenefile).string());
	Engine::Get().SwitchRenderer(0);

	m_selectedNode = nullptr;
	Event::OnWindowResize.Broadcast(Engine::GetMainWindow()->GetWidth(), Engine::GetMainWindow()->GetHeight());
}

void Editor::HandleInput()
{
}

void Editor::PreBeginFrame()
{
	if (!m_sceneToLoad.empty()) {
		LoadScene(m_sceneToLoad);
		m_sceneToLoad = "";
	}
}
