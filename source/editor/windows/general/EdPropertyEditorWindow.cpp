#include "pch.h"
#include "EdPropertyEditorWindow.h"

#include "assets/PodEditor.h"
#include "ecs_universe/ComponentsDb.h"
#include "editor/DataStrings.h"
#include "editor/Editor.h"
#include "editor/imgui/ImAssetSlot.h"
#include "editor/imgui/ImguiUtil.h"
#include "editor/imgui/ImGuizmo.h"
#include "editor/windows/general/EdEcsOutlinerWindow.h"
#include "engine/profiler/ProfileScope.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/Node.h"
#include "universe/Universe.h"

#include <glm/gtc/type_ptr.hpp>

inline float* FromVec4(glm::vec4& vec4)
{
	return glm::value_ptr(vec4);
}

namespace ed {

namespace {

	using namespace PropertyFlags;

	template<typename T>
	constexpr bool CanOpenFromFile = refl::IsValidPod<T>;

	template<typename T>
	constexpr bool IsJsonLoadable
		= std::is_same_v<Material, T> || std::is_same_v<Shader, T> || std::is_same_v<Sampler, T>;


	struct ReflectionToImguiVisitor {
		int32 depth{ 0 };

		std::string nameBuf;
		const char* name;
		bool fullDisplayMat4{ false };

		DirtyFlagset dirtyFlags{};

		int32 id{ 1 };

		void* currentObject{ nullptr };
		const Property* currentProperty{ nullptr };

		bool didEditFlag{ false };

		void Begin(void* objPtr, const ReflClass& cl) { currentObject = objPtr; }
		void End(const ReflClass&) {}

		void PreProperty(const Property& p)
		{
			ImGui::PushID(id++);
			nameBuf = p.GetName();
			name = nameBuf.c_str();
			currentProperty = &p;
		}

		void PostProperty(const Property& p) { ImGui::PopID(); }

		template<typename T>
		void operator()(T& t, const Property& p)
		{
			if (p.HasFlags(Hidden)) {
				return;
			}
			if (!p.HasFlags(NoEdit)) {
				if (Inner(t, p)) {
					didEditFlag = true;
					if (p.GetDirtyFlagIndex() >= 0) {
						dirtyFlags.set(p.GetDirtyFlagIndex());
					}
					dirtyFlags.set(Node::DF::Properties);
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
				return ImGui::ColorEdit3(name, glm::value_ptr(t),
					ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_HDR
						| ImGuiColorEditFlags_Float);
			}

			return ImGui::DragFloat3(name, glm::value_ptr(t), 0.01f);
		}

		bool Inner(glm::vec4& t, const Property& p)
		{
			if (p.HasFlags(PropertyFlags::Color)) {
				return ImGui::ColorEdit4(name, glm::value_ptr(t),
					ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_HDR
						| ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
			}

			return ImGui::DragFloat4(name, glm::value_ptr(t), 0.01f);
		}

		bool Inner(std::string& ref, const Property& p)
		{
			if (p.HasFlags(PropertyFlags::Multiline)) {
				bool val = ImGui::InputTextMultiline(name, &ref, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
				return val;
			}

			return ImGui::InputText(name, &ref);
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

			std::array<std::string, 4> buffer;
			buffer[0] = p.GetNameStr() + "[0]";
			buffer[1] = p.GetNameStr() + "[1]";
			buffer[2] = p.GetNameStr() + "[2]";
			buffer[3] = p.GetNameStr() + "[3]";


			edited |= ImGui::DragFloat4(buffer[0].c_str(), FromVec4(rowMajor[0]), 0.01f);
			edited |= ImGui::DragFloat4(buffer[1].c_str(), FromVec4(rowMajor[1]), 0.01f);
			edited |= ImGui::DragFloat4(buffer[2].c_str(), FromVec4(rowMajor[2]), 0.01f);
			edited |= ImGui::DragFloat4(buffer[3].c_str(), FromVec4(rowMajor[3]), 0.01f);
			ImGui::Separator();
			if (edited) {
				t = glm::transpose(rowMajor);
			}
			return edited;
		}

		template<typename PodType>
		bool PodDropTarget(PodHandle<PodType>& pod)
		{
			bool result = false;
			if (PodEntry* entry = ImEd::AcceptTypedPodDrop<PodType>()) {
				pod.uid = entry->uid;
				dirtyFlags.set(Node::DF::Properties);
				result = true;
			}
			return result;
		}

		template<typename PodType>
		bool InjectPodCode(PodHandle<PodType>& pod, const Property& p, bool isInVector = false, uint64 extraId = 1)
		{
			auto entry = AssetHandlerManager::GetEntry(pod);
			const char* thisName = isInVector ? entry->name.c_str() : name;
			return ImEd::AssetSlot(thisName, pod);
		}

		template<typename PodType>
		bool Inner(PodHandle<PodType>& pod, const Property& p)
		{
			return InjectPodCode(pod, p);
		}

		template<typename T>
		bool Inner(std::vector<PodHandle<T>>& t, const Property& p)
		{
			bool result = false;
			if (ImGui::CollapsingHeader(name)) {
				ImGui::Indent();
				int32 index = 0;
				for (auto& handle : t) {
					ImGui::PushID(index);
					++index;

					result |= InjectPodCode(handle, p, true, index * 1024);

					ImGui::PopID();
				}
				ImGui::Unindent();
			}
			return result;
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

	template<typename T>
	void SaveAs(PodHandle<T>& handle, fs::path& path)
	{
		using json = nlohmann::json;
		path.replace_extension(".json");

		json j;
		refltools::PropertiesToJson(const_cast<T*>(handle.Lock()), j);
		std::ofstream file(path);
		file << std::setw(4) << j;
	}


} // namespace

void PropertyEditorWindow::ImguiDraw()
{
	PROFILE_SCOPE(Editor);

	Entity ent = EcsOutlinerWindow::selected;

	if (!ent || !ent.registry->valid(ent.entity)) {
		ImGui::Text("No entity selected.");
		return;
	}


	Run_BaseProperties(ent);
	ImGui::Separator();
	Run_ImGuizmo(ent);


	Run_Components(ent);

	// m_prevNode = node;
}


void PropertyEditorWindow::Run_BaseProperties(Entity node)
{
	std::string name = node->name;

	if (ImGui::InputText("Name", &name)) {
		node->name = name;
	}

	// glm::vec3 location;
	// glm::vec3 eulerPyr;
	// glm::vec3 scale;

	// if (m_localMode) {
	//	location = node->GetNodePositionLCS();
	//	eulerPyr = node->GetNodeEulerAnglesLCS();
	//	scale = node->GetNodeScaleLCS();
	//}
	// else {
	//	location = node->GetNodePositionWCS();
	//	eulerPyr = node->GetNodeEulerAnglesWCS();
	//	scale = node->GetNodeScaleWCS();
	//}

	// const auto UpdateLookAtReference = [&]() {
	//	auto fwd = m_localMode ? node->GetNodeForwardLCS() : node->GetNodeForwardWCS();
	//	auto lookAt = location + fwd;
	//	m_lookAtPos = lookAt;
	//};

	// if (m_prevNode != node) {
	//	UpdateLookAtReference();
	//}

	// if (ImGui::DragFloat3("Position", glm::value_ptr(location), 0.01f)) {
	//	m_localMode ? node->SetNodePositionLCS(location) : node->SetNodePositionWCS(location);
	//}
	// if (ImGui::BeginPopupContextItem("PositionPopup")) {
	//	if (ImGui::MenuItem("Reset##1")) {
	//		m_localMode ? node->SetNodePositionLCS({}) : node->SetNodePositionWCS({});
	//	}
	//	ImGui::EndPopup();
	//}
	// if (!m_lookAtMode) {
	//	if (ImGui::DragFloat3("Rotation", glm::value_ptr(eulerPyr), 0.1f)) {
	//		auto deltaAxis = eulerPyr - (m_localMode ? node->GetNodeEulerAnglesLCS() : node->GetNodeEulerAnglesWCS());
	//		if (ImGui::IsAnyMouseDown()) {
	//			// On user drag use quat diff, prevents gimbal locks while dragging
	//			m_localMode
	//				? node->SetNodeOrientationLCS(glm::quat(glm::radians(deltaAxis)) * node->GetNodeOrientationLCS())
	//				: node->SetNodeOrientationWCS(glm::quat(glm::radians(deltaAxis)) * node->GetNodeOrientationWCS());
	//		}
	//		else {
	//			// On user type set pyr directly, prevents the axis from flickering
	//			m_localMode ? node->SetNodeEulerAnglesLCS(eulerPyr) : node->SetNodeEulerAnglesWCS(eulerPyr);
	//		}
	//	}
	//}
	// else {
	//	ImGui::DragFloat3("Look At", glm::value_ptr(m_lookAtPos), 0.1f);
	//	ImEd::HelpTooltipInline(
	//		"Look at will lock lookat position of the selected node for as long as it is active. This way you can "
	//		"adjust the position of your node while keeping the lookat position fixed.");
	//	m_localMode ? node->SetNodeLookAtLCS(m_lookAtPos) : node->SetNodeLookAtWCS(m_lookAtPos);
	//}

	// if (ImGui::BeginPopupContextItem("RotatePopup")) {
	//	if (ImGui::MenuItem("Reset##2")) {
	//		m_localMode ? node->SetNodeOrientationLCS(glm::identity<glm::quat>())
	//					: node->SetNodeOrientationWCS(glm::identity<glm::quat>());
	//	}
	//	if (ImGui::MenuItem("Look At", nullptr, m_lookAtMode)) {
	//		m_lookAtMode = !m_lookAtMode;
	//		if (m_lookAtMode) {
	//			UpdateLookAtReference();
	//		}
	//	}
	//	ImGui::EndPopup();
	//}

	// if (!m_lockedScale) {
	//	if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f)) {
	//		m_localMode ? node->SetNodeScaleLCS(scale) : node->SetNodeScaleWCS(scale);
	//	}
	//}
	// else {
	//	glm::vec3 newScale = m_localMode ? node->GetNodeScaleLCS() : node->GetNodeScaleWCS();
	//	if (ImGui::DragFloat3("Locked Scale", glm::value_ptr(newScale), 0.01f)) {
	//		glm::vec3 initialScale = m_localMode ? node->GetNodeScaleLCS() : node->GetNodeScaleWCS();

	//		float ratio = 1.f;
	//		if (!math::equals(newScale.x, initialScale.x)) {
	//			ratio = newScale.x / initialScale.x;
	//		}
	//		else if (!math::equals(newScale.y, initialScale.y)) {
	//			ratio = newScale.y / initialScale.y;
	//		}
	//		else if (!math::equals(newScale.z, initialScale.z)) {
	//			ratio = newScale.z / initialScale.z;
	//		}

	//		ratio += 0.00001f;
	//		m_localMode ? node->SetNodeScaleLCS(initialScale * ratio) : node->SetNodeScaleWCS(initialScale * ratio);
	//	}
	//}

	// if (ImGui::BeginPopupContextItem("ScalePopup")) {
	//	if (ImGui::MenuItem("Reset##3")) {
	//		m_localMode ? node->SetNodeScaleLCS(glm::vec3(1.f)) : node->SetNodeScaleWCS(glm::vec3(1.f));
	//	}
	//	if (ImGui::MenuItem("Lock", nullptr, m_lockedScale)) {
	//		m_lockedScale = !m_lockedScale;
	//	}
	//	ImGui::EndPopup();
	//}

	// if (ImGui::Checkbox("Local Mode", &m_localMode)) {
	//	UpdateLookAtReference();
	//}
	// ImEd::HelpTooltipInline("Toggles local/global space for TRS and transform matrix editing.");
	// ImGui::SameLine(0.f, 16.f);
	ImGui::Checkbox("Display Matrix", &m_displayMatrix);
	ImEd::HelpTooltipInline("Toggles visiblity and editing of matricies as a row major table.");

	if (m_displayMatrix) {

		glm::mat4 matrix = m_localMode ? node->local().transform : node->world().transform;

		// to row major
		auto rowMajor = glm::transpose(matrix);

		bool edited = false;

		edited |= ImGui::DragFloat4("mat.row[0]", glm::value_ptr(rowMajor[0]), 0.01f);
		edited |= ImGui::DragFloat4("mat.row[1]", glm::value_ptr(rowMajor[1]), 0.01f);
		edited |= ImGui::DragFloat4("mat.row[2]", glm::value_ptr(rowMajor[2]), 0.01f);
		edited |= ImGui::DragFloat4("mat.row[3]", glm::value_ptr(rowMajor[3]), 0.01f);
		if (edited) {
			m_localMode ? node->SetNodeTransformLCS(glm::transpose(rowMajor))
						: node->SetNodeTransformWCS(glm::transpose(rowMajor));
		}
	}
}

void PropertyEditorWindow::Run_Components(Entity entity)
{
	auto& reg = *entity.registry;
	auto ent = entity.entity;

	ReflectionToImguiVisitor visitor;
	visitor.fullDisplayMat4 = m_displayMatrix;

	auto map = ComponentsDb::Z_GetTypes();

	ComponentsDb::VisitWithType(entity, [&](const ComponentMetaEntry& comp) {
		map.erase(comp.entType);

		ImGui::PushID(comp.entType);
		auto& cl = *comp.clPtr;
		auto data = comp.get(reg, ent);
		CLOG_ERROR(!data, "Visited with type that was not present in the entity.");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);


		bool remove = ImGui::Button(U8(FA_TIMES));
		if (remove) {
			comp.safeRemove(reg, ent);
			ImGui::PopID();
			return;
		}


		ImGui::SameLine();
		if (ImGui::CollapsingHeader(cl.GetNameStr().c_str())) {
			ImGui::Indent(44.f);
			refltools::CallVisitorOnEveryPropertyEx(data, cl, visitor);
			ImGui::Unindent(44.f);

			if (visitor.didEditFlag) {
				comp.markDirty(reg, ent);
				visitor.didEditFlag = false;
			}
		}

		ImGui::PopID();
	});

	if (map.size()) {
		if (ImGui::BeginPopupContextWindow()) {
			for (auto& [id, entry] : map) {
				if (ImGui::MenuItem(entry.clPtr->GetNameStr().c_str())) {
					entry.emplace(reg, ent);
				}
			}
			ImGui::EndPopup();
		}
	}
}

void PropertyEditorWindow::Run_ImGuizmo(Entity node)
{
	auto world = Universe::GetMainWorld();
	auto camera = world->GetActiveCamera();

	if (!camera) {
		return;
	}

	auto cameraView = camera->GetViewMatrix();
	auto cameraProj = camera->GetProjectionMatrix();

	cameraProj[1][1] *= -1.0;

	auto nodeMatrix = node->world().transform;

	// auto i = glm::identity<glm::mat4>();
	// ImGuizmo::DrawGrid(glm::value_ptr(cameraView), glm::value_ptr(cameraProj), glm::value_ptr(i), 10.f);

	ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
		static_cast<ImGuizmo::OPERATION>(m_manipMode.op), static_cast<ImGuizmo::MODE>(m_manipMode.mode),
		glm::value_ptr(nodeMatrix));

	if (ImGuizmo::IsUsing()) {
		node->SetNodeTransformWCS(nodeMatrix);
	}
}

// HACK:
// Declared in EdGenericAssetEditorWindow.h but used here to avoid duplicating the imgui visitor.
void GenericImguiDrawEntry(PodEntry* entry)
{
	ReflectionToImguiVisitor visitor;
	visitor.fullDisplayMat4 = false;
	refltools::CallVisitorOnEveryProperty(entry->ptr.get(), visitor);
	if (visitor.didEditFlag) {
		PodEditorBase::CommitUpdate(entry->uid, {});
	}
}

bool GenericImguiDrawClass(void* object, const ReflClass& cl)
{
	ReflectionToImguiVisitor visitor;
	visitor.fullDisplayMat4 = false;
	refltools::CallVisitorOnEveryPropertyEx(object, cl, visitor);
	return visitor.didEditFlag;
}
} // namespace ed
