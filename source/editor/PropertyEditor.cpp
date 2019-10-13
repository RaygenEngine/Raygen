#include "pch/pch.h"

#include "editor/PropertyEditor.h"

#include "asset/PodHandle.h"
#include "world/nodes/Node.h"
#include "editor/Editor.h"
#include "system/Engine.h"
#include "reflection/ReflectionTools.h"

#include "editor/imgui/ImguiUtil.h"


namespace {

using namespace PropertyFlags;

struct ReflectionToImguiVisitor {
	int32 depth{ 0 };
	std::string path{};

	std::set<std::string> objNames;

	std::string nameBuf;
	const char* name;

	Node* node;

	DirtyFlagset dirtyFlags{};

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
			return ImGui::ColorEdit3(name, ImUtil::FromVec3(t), ImGuiColorEditFlags_DisplayHSV);
		}

		return ImGui::DragFloat3(name, ImUtil::FromVec3(t), 0.01f);
	}

	bool Inner(glm::vec4& t, const Property& p)
	{
		if (p.HasFlags(PropertyFlags::Color)) {
			return ImGui::ColorEdit4(name, ImUtil::FromVec4(t), ImGuiColorEditFlags_DisplayHSV);
		}

		return ImGui::DragFloat4(name, ImUtil::FromVec4(t), 0.01f);
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

void PropertyEditor::Inject(Node* node)
{
	ImGui::Indent();
	if (!node->IsRoot()) {
		Run_BaseProperties(node);

		ImGui::Separator();

		Run_ContextActions(node);

		ImGui::Separator();
	}

	Run_ReflectedProperties(node);
	ImGui::Unindent();
	ImGui::Spacing();
}

void PropertyEditor::Run_BaseProperties(Node* node)
{
	std::string name = node->GetName();
	if (ImGui::InputText("Name", &name)) {
		node->SetName(name);
	}

	glm::vec3 location;
	glm::vec3 eulerPyr;
	glm::vec3 scale;

	if (m_localMode) {
		location = node->GetLocalTranslation();
		eulerPyr = node->GetLocalPYR();
		scale = node->GetLocalScale();
	}
	else {
		location = node->GetWorldTranslation();
		eulerPyr = node->GetWorldPYR();
		scale = node->GetWorldScale();
	}

	if (ImGui::DragFloat3("Position", ImUtil::FromVec3(location), 0.01f)) {
		m_localMode ? node->SetLocalTranslation(location) : node->SetWorldTranslation(location);
	}

	if (ImGui::DragFloat3("Rotation", ImUtil::FromVec3(eulerPyr), 0.1f)) {
		m_localMode ? node->SetLocalPYR(eulerPyr) : node->SetWorldPYR(eulerPyr);
	}

	if (ImGui::DragFloat3("Scale", ImUtil::FromVec3(scale), 0.01f)) {
		m_localMode ? node->SetLocalScale(scale) : node->SetWorldScale(scale);
	}

	ImGui::Checkbox("Local Mode", &m_localMode);
}

void PropertyEditor::Run_ContextActions(Node* node)
{
	bool wasLastSplitter = false;
	auto v = Engine::GetEditor()->m_nodeContextActions->GetActions(node);

	int32 splitters = 0;

	ImGui::Indent();

	for (auto& action : v) {
		if (action.IsSplitter()) {
			wasLastSplitter = true;
			continue;
		}

		ImGui::SameLine();
		if (wasLastSplitter) {
			if (splitters == 2) {
				ImGui::Text("");
				splitters = 0;
			}
			else {
				ImGui::Text("|");
				ImGui::SameLine();
				splitters++;
			}
		}
		wasLastSplitter = false;
		if (!action.IsSplitter() && ImGui::Button(action.name)) {
			action.function(node);
		}
	}
	ImGui::Unindent();
}

void PropertyEditor::Run_ReflectedProperties(Node* node)
{
	ReflectionToImguiVisitor visitor;
	visitor.node = node;

	refltools::CallVisitorOnEveryProperty(node, visitor);

	node->SetDirtyMultiple(visitor.dirtyFlags);
}
