#include "pch/pch.h"

#include "editor/PropertyEditor.h"

#include "asset/PodHandle.h"
#include "world/nodes/Node.h"
#include "editor/Editor.h"
#include "system/Engine.h"
#include "reflection/ReflectionTools.h"
#include "core/MathAux.h"

#include "editor/imgui/ImguiUtil.h"
#include "editor/DataStrings.h"


namespace {

using namespace PropertyFlags;

struct ReflectionToImguiVisitor {
	int32 depth{ 0 };

	std::string nameBuf;
	const char* name;
	bool fullDisplayMat4{ false };
	Node* node;

	DirtyFlagset dirtyFlags{};

	int32 id{ 1 };

	// TODO: a little hack for mass material editing in vectors, should be replaced when and if we do actual asset
	// editors.
	bool massEditMaterials{ false };
	std::vector<PodHandle<MaterialPod>>* massEditMaterialVector{ nullptr };
	template<typename T>
	void MassMaterialEdit(T& t, const Property& p)
	{
		if (!massEditMaterialVector) {
			massEditMaterials = false;
			return;
		}
		for (auto& materialHandle : *massEditMaterialVector) {
			p.GetRef<T>(const_cast<MaterialPod*>(materialHandle.Lock())) = t;
		}
	}

	template<typename T>
	void MassMaterialEdit(PodHandle<T>& t, const Property& p)
	{
	}
	template<typename T>
	void MassMaterialEdit(std::vector<PodHandle<T>>& t, const Property& p)
	{
	}
	template<>
	void MassMaterialEdit(MetaEnumInst& t, const Property& p)
	{
		if (!massEditMaterialVector) {
			massEditMaterials = false;
			return;
		}
		for (auto& materialHandle : *massEditMaterialVector) {
			p.GetEnumRef(const_cast<MaterialPod*>(materialHandle.Lock())).SetValue(t.GetValue());
		}
	}
	// End of mass edit material


	void PreProperty(const Property& p)
	{
		ImGui::PushID(id++);
		nameBuf = p.GetName();
		name = nameBuf.c_str();
	}

	void PostProperty(const Property& p) { ImGui::PopID(); }

	template<typename T>
	void operator()(T& t, const Property& p)
	{
		if (p.HasFlags(PropertyFlags::Hidden)) {
			return;
		}
		if (!p.HasFlags(NoEdit)) {
			if (Inner(t, p)) {
				if (p.GetDirtyFlagIndex() >= 0) {
					dirtyFlags.set(p.GetDirtyFlagIndex());
				}
				dirtyFlags.set(Node::DF::Properties);
				if (massEditMaterials) {
					MassMaterialEdit<T>(t, p);
				}
			}
		}
		else { // No edit version, just copy twice
			auto copy = t;
			if (Inner(t, p)) {
				t = copy;
			}
		}
	}

	bool Inner(int32& t, const Property& p) { return ImGui::DragInt(name, &t, 0.1f); }

	bool Inner(bool& t, const Property& p) { return ImGui::Checkbox(name, &t); }

	bool Inner(float& t, const Property& p)
	{
		if (p.HasFlags(Rads)) {
			auto degrees = glm::degrees(t);
			if (ImGui::DragFloat(name, &degrees, 0.01f)) {
				t = glm::radians(degrees);
				return true;
			}
			TEXT_TOOLTIP("Converted to degrees for the editor, actual value is: {}", t);
			return false;
		}
		return ImGui::DragFloat(name, &t, 0.01f);
	}

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
	bool PodDropTarget(PodHandle<PodType>& pod)
	{
		bool result = false;
		if (ImGui::BeginDragDropTarget()) {
			std::string payloadTag = "POD_UID_" + std::to_string(ctti::type_id<PodType>().hash());
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag.c_str())) {
				assert(payload->DataSize == sizeof(size_t));
				size_t uid = *reinterpret_cast<size_t*>(payload->Data);
				pod.podId = uid;
				dirtyFlags.set(Node::DF::Properties);
				result = true;
			}
			ImGui::EndDragDropTarget();
		}
		return result;
	}

	template<typename PodType>
	bool Inner(PodHandle<PodType>& pod, const Property& p)
	{
		if (!pod.HasBeenAssigned()) {
			std::string s = "Unitialised handle: " + p.GetNameStr();
			ImGui::Text(s.c_str());
			return false;
		}

		auto entry = AssetManager::GetEntry(pod);

		auto str = entry->name;
		bool open = ImGui::CollapsingHeader(name);
		bool result = PodDropTarget(pod);
		TEXT_TOOLTIP("Pod Internal Path:\n{}\n", AssetManager::GetPodUri(pod));
		if (depth == 0) {
			Editor::CollapsingHeaderTooltip(help_PropPodEditing);
		}

		if (open) {
			ImGui::PushID(static_cast<uint32>(entry->uid) * 1024);
			// GenerateUniqueName(p);


			depth++;
			ImGui::Indent();

			refltools::CallVisitorOnEveryProperty(const_cast<PodType*>(pod.Lock()), *this);

			ImGui::Unindent();
			depth--;
			ImGui::PopID();
		}
		return result;
	}

	template<typename T>
	bool Inner(T& t, const Property& p)
	{
		std::string s = "unhandled property: " + p.GetNameStr();
		ImGui::Text(s.c_str());
		return false;
	}

	bool Inner(glm::mat4& t, const Property& p)
	{
		if (!fullDisplayMat4) {
			std::string s = "mat4: " + p.GetNameStr();

			ImGui::Text(s.c_str());
			TEXT_TOOLTIP("Enable matrix editing by clicking on display matrix under node transform.\n\n{}:{}",
				p.GetNameStr(), parsingaux::mat4_to_string(t));
			return false;
		}

		// display in row major
		auto rowMajor = glm::transpose(t);
		bool edited = false;

		std::array<std::string, 4> buffers;
		buffers[0] = p.GetNameStr() + "[0]";
		buffers[1] = p.GetNameStr() + "[1]";
		buffers[2] = p.GetNameStr() + "[2]";
		buffers[3] = p.GetNameStr() + "[3]";


		edited |= ImGui::DragFloat4(buffers[0].c_str(), ImUtil::FromVec4(rowMajor[0]), 0.01f);
		edited |= ImGui::DragFloat4(buffers[1].c_str(), ImUtil::FromVec4(rowMajor[1]), 0.01f);
		edited |= ImGui::DragFloat4(buffers[2].c_str(), ImUtil::FromVec4(rowMajor[2]), 0.01f);
		edited |= ImGui::DragFloat4(buffers[3].c_str(), ImUtil::FromVec4(rowMajor[3]), 0.01f);
		ImGui::Separator();
		if (edited) {
			t = glm::transpose(rowMajor);
		}
		return edited;
	}

	template<typename T>
	bool Inner(std::vector<PodHandle<T>>& t, const Property& p)
	{
		if (ImGui::CollapsingHeader(name)) {
			ImGui::Indent();
			int32 index = 0;
			for (auto& handle : t) {
				ImGui::PushID(index);
				++index;

				std::string finalName = AssetManager::GetEntry(handle)->name;

				bool r = ImGui::CollapsingHeader(finalName.c_str());
				PodDropTarget(handle);
				TEXT_TOOLTIP("{}", AssetManager::GetPodUri(handle));
				if (r) {
					ImGui::PushID(index * 1024 * 1024);
					ImGui::Indent();
					refltools::CallVisitorOnEveryProperty(const_cast<T*>(handle.Lock()), *this);
					ImGui::Unindent();
					ImGui::PopID();
				}


				ImGui::PopID();
			}
			ImGui::Unindent();
		}
		return false;
	}

	template<>
	bool Inner(std::vector<PodHandle<MaterialPod>>& t, const Property& p)
	{
		if (ImGui::CollapsingHeader(name)) {
			ImGui::Indent();
			int32 index = 0;
			bool isThisMassEditing = false;
			bool shouldReloadMaterials = false;
			if (massEditMaterialVector == nullptr) {
				ImGui::Checkbox("Mass Edit Materials", &massEditMaterials);
				Editor::HelpTooltipInline(help_PropMassEditMats);
				ImGui::SameLine();

				shouldReloadMaterials = ImGui::Button("Reload Materials");
				Editor::HelpTooltipInline(help_PropMassRestoreMats);
				if (massEditMaterials) {
					massEditMaterialVector = &t;
					isThisMassEditing = true;
				}
			}


			for (auto& handle : t) {
				ImGui::PushID(index);
				++index;

				std::string finalName = AssetManager::GetEntry(handle)->name;

				bool r = ImGui::CollapsingHeader(finalName.c_str());
				PodDropTarget(handle);
				TEXT_TOOLTIP("{}", AssetManager::GetPodUri(handle));
				if (r) {
					ImGui::PushID(index * 1024 * 1024);
					ImGui::Indent();
					refltools::CallVisitorOnEveryProperty(const_cast<MaterialPod*>(handle.Lock()), *this);
					ImGui::Unindent();
					ImGui::PopID();
				}
				if (shouldReloadMaterials) {
					AssetManager::Reload(handle);
				}

				ImGui::PopID();
			}

			if (isThisMassEditing) {
				massEditMaterialVector = nullptr;
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

		bool edited = false;

		if (ImGui::BeginCombo(
				name, str.c_str())) // The second parameter is the label previewed before opening the combo.
		{
			for (auto& [enumStr, value] : enumMeta.GetStringsToValues()) {
				bool selected = (currentValue == value);
				if (ImGui::Selectable(enumStr.c_str(), &selected)) {
					t.SetValue(value);
					edited = true;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return edited;
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
	if (ImGui::BeginPopupContextItem("PositionPopup")) {
		if (ImGui::MenuItem("Reset##1")) {
			m_localMode ? node->SetLocalTranslation({}) : node->SetWorldTranslation({});
		}
		ImGui::EndPopup();
	}
	if (ImGui::DragFloat3("Rotation", ImUtil::FromVec3(eulerPyr), 0.1f)) {
		auto deltaAxis = eulerPyr - (m_localMode ? node->GetLocalPYR() : node->GetWorldPYR());
		if (ImGui::IsAnyMouseDown()) {
			// On user drag use quat diff, prevents gimbal locks while dragging
			m_localMode ? node->SetLocalOrientation(glm::quat(glm::radians(deltaAxis)) * node->GetLocalOrientation())
						: node->SetWorldOrientation(glm::quat(glm::radians(deltaAxis)) * node->GetWorldOrientation());
		}
		else {
			// On user type set pyr directly, prevents the axis from flickering
			m_localMode ? node->SetLocalPYR(eulerPyr) : node->SetWorldPYR(eulerPyr);
		}
	}
	if (ImGui::BeginPopupContextItem("RotatePopup")) {
		if (ImGui::MenuItem("Reset##2")) {
			m_localMode ? node->SetLocalOrientation(glm::identity<glm::quat>())
						: node->SetWorldOrientation(glm::identity<glm::quat>());
		}
		ImGui::EndPopup();
	}

	static bool lockedScale = false;
	if (!lockedScale) {
		if (ImGui::DragFloat3("Scale", ImUtil::FromVec3(scale), 0.01f)) {
			m_localMode ? node->SetLocalScale(scale) : node->SetWorldScale(scale);
		}
	}
	else {
		glm::vec3 newScale = m_localMode ? node->GetLocalScale() : node->GetWorldScale();
		if (ImGui::DragFloat3("Locked Scale", ImUtil::FromVec3(newScale), 0.01f)) {
			glm::vec3 initialScale = m_localMode ? node->GetLocalScale() : node->GetWorldScale();

			float ratio = 1.f;
			if (!math::EpsilonEqualsValue(newScale.x, initialScale.x) && !math::EpsilonEqualsZero(initialScale.x)) {
				ratio = newScale.x / initialScale.x;
			}
			else if (!math::EpsilonEqualsValue(newScale.y, initialScale.y)
					 && !math::EpsilonEqualsZero(initialScale.y)) {
				ratio = newScale.y / initialScale.y;
			}
			else if (!math::EpsilonEqualsZero(initialScale.z)) {
				ratio = newScale.z / initialScale.z;
			}
			m_localMode ? node->SetLocalScale(initialScale * ratio) : node->SetWorldScale(initialScale * ratio);
		}
	}

	if (ImGui::BeginPopupContextItem("ScalePopup")) {
		if (ImGui::MenuItem("Reset##3")) {
			m_localMode ? node->SetLocalScale(glm::vec3(1.f)) : node->SetWorldScale(glm::vec3(1.f));
		}
		if (ImGui::MenuItem("Lock", nullptr, lockedScale)) {
			lockedScale = !lockedScale;
		}
		ImGui::EndPopup();
	}

	ImGui::Checkbox("Local Mode", &m_localMode);
	Editor::HelpTooltipInline("Toggles local/global space for TRS and transform matrix editing.");
	ImGui::SameLine(0.f, 16.f);
	ImGui::Checkbox("Display Matrix", &m_displayMatrix);
	Editor::HelpTooltipInline("Toggles visiblity and editing of matricies as a row major table.");

	if (m_displayMatrix) {

		glm::mat4 matrix = m_localMode ? node->GetLocalMatrix() : node->GetWorldMatrix();

		// to row major
		auto rowMajor = glm::transpose(matrix);

		bool edited = false;

		edited |= ImGui::DragFloat4("mat.row[0]", ImUtil::FromVec4(rowMajor[0]), 0.01f);
		edited |= ImGui::DragFloat4("mat.row[1]", ImUtil::FromVec4(rowMajor[1]), 0.01f);
		edited |= ImGui::DragFloat4("mat.row[2]", ImUtil::FromVec4(rowMajor[2]), 0.01f);
		edited |= ImGui::DragFloat4("mat.row[3]", ImUtil::FromVec4(rowMajor[3]), 0.01f);
		if (edited) {
			m_localMode ? node->SetLocalMatrix(glm::transpose(rowMajor))
						: node->SetWorldMatrix(glm::transpose(rowMajor));
		}
	}
}

void PropertyEditor::Run_ContextActions(Node* node)
{
	auto v = Engine::GetEditor()->m_nodeContextActions->GetActions(node, false);

	ImGui::Indent();

	ImGuiStyle& style = ImGui::GetStyle();
	float maxWidth = ImGui::GetWindowContentRegionWidth();

	float indentWidth = ImGui::GetCursorPosX();
	float totalWidth = 0.f;
	float lastWidth = 0.f;

	for (auto& action : v) {
		if (action.IsSplitter()) {
			ImGui::SameLine();
			ImGui::Text(" ");
			continue;
		}

		float predictedWidth = strlen(action.name) * 7.f + style.ItemInnerSpacing.x * 2 + style.ItemSpacing.x;

		if (totalWidth + predictedWidth + indentWidth < maxWidth) {
			ImGui::SameLine();
		}
		else {
			totalWidth = 0.f;
		}

		if (ImGui::Button(action.name)) {
			action.function(node);
		}
		lastWidth = ImGui::GetItemRectSize().x;
		totalWidth += lastWidth;
		indentWidth = ImGui::GetCursorPosX();
	}

	ImGui::Unindent();
}

void PropertyEditor::Run_ReflectedProperties(Node* node)
{
	ReflectionToImguiVisitor visitor;
	visitor.node = node;
	visitor.fullDisplayMat4 = m_displayMatrix;
	visitor.massEditMaterials = m_massEditMaterials;
	refltools::CallVisitorOnEveryProperty(node, visitor);

	m_massEditMaterials = visitor.massEditMaterials;
	node->SetDirtyMultiple(visitor.dirtyFlags);
}
