#include "EdPropertyEditorWindow.h"

#include "assets/PodEditor.h"
#include "editor/EdClipboardOp.h"
#include "editor/imgui/ImAssetSlot.h"
#include "editor/windows/general/EdOutlinerWindow.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "reflection/ReflectionTools.h"
#include "universe/ComponentsDb.h"
#include "universe/World.h"

inline float* FromVec4(glm::vec4& vec4)
{
	return glm::value_ptr(vec4);
}

namespace ed {

namespace {
	template<typename T>
	concept CanBeClamped = requires(T a)
	{
		{
			glm::clamp(a, Property::Editor_MinMaxRetType<T>{}, Property::Editor_MinMaxRetType<T>{})
			} -> std::convertible_to<T>;
	};

	static_assert(CanBeClamped<float>);
	static_assert(CanBeClamped<int32>);
	static_assert(CanBeClamped<glm::vec3>);
	static_assert(!CanBeClamped<std::string>);


	using namespace PropertyFlags;

	struct ReflectionToImguiVisitor {
		int32 depth{ 0 };

		std::string nameBuf;
		const char* name;
		bool fullDisplayMat4{ false };

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
					if constexpr (CanBeClamped<T>) {
						if (p.HasFlags(EditorClamp) && !ImGui::GetIO().KeyCtrl) {
							t = glm::clamp(t, p.Editor_GetMin<T>(), p.Editor_GetMax<T>());
						}
					}
					didEditFlag = true;
				}
			}
			else { // No edit version, just copy twice
				auto copy = t;
				if (Inner(t, p)) {
					t = copy;
				}
			}
		}

		bool Inner(int32& t, const Property& p) { return ImGui::DragInt(name, &t, p.Editor_GetSpeed() * 0.1f); }

		bool Inner(bool& t, const Property& p) { return ImGui::Checkbox(name, &t); }

		bool Inner(float& t, const Property& p)
		{
			if (p.HasFlags(Rads)) {
				auto degrees = glm::degrees(t);
				if (ImGui::DragFloat(name, &degrees, p.Editor_GetSpeed() * 0.01f)) {
					t = glm::radians(degrees);
					return true;
				}
				TEXT_TOOLTIP("Converted to degrees for the editor, actual value is: {}", t);
				return false;
			}
			return ImGui::DragFloat(name, &t, p.Editor_GetSpeed() * 0.01f);
		}

		bool Inner(glm::vec3& t, const Property& p)
		{
			if (p.HasFlags(PropertyFlags::Color)) {
				return ImGui::ColorEdit3(name, glm::value_ptr(t),
					ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_HDR
						| ImGuiColorEditFlags_Float);
			}

			return ImGui::DragFloat3(name, glm::value_ptr(t), p.Editor_GetSpeed() * 0.01f);
		}

		bool Inner(glm::vec4& t, const Property& p)
		{
			if (p.HasFlags(PropertyFlags::Color)) {
				return ImGui::ColorEdit4(name, glm::value_ptr(t),
					ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_HDR
						| ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
			}

			return ImGui::DragFloat4(name, glm::value_ptr(t), p.Editor_GetSpeed() * 0.01f);
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

			std::array<std::string, 4> buffers;
			buffers[0] = p.GetNameStr() + "[0]";
			buffers[1] = p.GetNameStr() + "[1]";
			buffers[2] = p.GetNameStr() + "[2]";
			buffers[3] = p.GetNameStr() + "[3]";


			edited |= ImGui::DragFloat4(buffers[0].c_str(), FromVec4(rowMajor[0]), p.Editor_GetSpeed() * 0.01f);
			edited |= ImGui::DragFloat4(buffers[1].c_str(), FromVec4(rowMajor[1]), p.Editor_GetSpeed() * 0.01f);
			edited |= ImGui::DragFloat4(buffers[2].c_str(), FromVec4(rowMajor[2]), p.Editor_GetSpeed() * 0.01f);
			edited |= ImGui::DragFloat4(buffers[3].c_str(), FromVec4(rowMajor[3]), p.Editor_GetSpeed() * 0.01f);
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
				result = true;
			}
			return result;
		}

		template<typename PodType>
		bool InjectPodCode(PodHandle<PodType>& pod, const Property& p, bool isInVector = false, uint64 extraId = 1)
		{
			auto entry = AssetRegistry::GetEntry(pod);
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


} // namespace

void PropertyEditorWindow::ImguiDraw()
{
	PROFILE_SCOPE(Editor);

	Entity ent = OutlinerWindow::selected;

	if (!ent) {
		ImGui::Text("No entity selected.");
		return;
	}


	Run_BaseProperties(ent);
	ImGui::Separator();
	Run_ImGuizmo(ent);


	Run_Components(ent);

	// m_prevNode = node;
}


void PropertyEditorWindow::Run_BaseProperties(Entity ent)
{
	std::string name = ent->name;

	if (ImGui::InputText("Name", &name)) {
		ent->name = name;
	}

	bool changed = false;
	auto tr = m_localMode ? ent->local() : ent->world();

	if (ImEd::TransformRun(tr, true, &m_lockedScale, &m_localMode, &m_displayMatrix, &m_lookAtMode, &m_lookAtPos)) {
		if (m_localMode) {
			ent->SetNodeTransformLCS(tr.transform);
		}
		else {
			ent->SetNodeTransformWCS(tr.transform);
		}
	}
}

void PropertyEditorWindow::Run_Components(Entity entity)
{
	if (ImGui::BeginChild("PropEditor_ComponentsChildWindow")) {
		ReflectionToImguiVisitor visitor;
		visitor.fullDisplayMat4 = m_displayMatrix;

		auto map = ComponentsDb::Z_GetTypes();

		ComponentsDb::VisitWithType(entity, [&](const ComponentMetaEntry& comp) {
			map.erase(comp.entType);

			ImGui::PushID(comp.entType);
			auto& cl = *comp.clPtr;
			auto data = comp.get(entity);
			CLOG_ERROR(!data, "Visited with type that was not present in the entity.");

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);


			bool remove = ImGui::Button(U8(FA_TIMES));
			if (remove) {
				comp.safeRemove(entity);
				ImGui::PopID();
				return;
			}


			ImGui::SameLine();
			std::string name = std::string(" " + std::string(U8(cl.GetIcon())) + "  " + cl.GetNameStr());

			if (ImGui::CollapsingHeader(name.data(), ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent(44.f);
				refltools::CallVisitorOnEveryPropertyEx(data, cl, visitor);
				ImGui::Unindent(44.f);

				if (visitor.didEditFlag) {
					comp.markDirty(entity);
					visitor.didEditFlag = false;
				}
			}

			ImGui::PopID();
		});

		if (map.size()) {
			if (ImGui::BeginPopupContextWindow(nullptr, ImGuiMouseButton_Right, false)) {
				for (auto& [id, entry] : map) {
					if (ImGui::MenuItem(entry.clPtr->GetNameStr().c_str())) {
						entry.emplace(entity);
					}
				}
				ImGui::EndPopup();
			}
		}
		ImGui::EndChild();
	}
}

void PropertyEditorWindow::Run_ImGuizmo(Entity node)
{
	auto& camera = EditorObject->edCamera;
	if (!EditorObject->m_currentWorld->IsStopped()) {
		return;
	}


	auto nodeMatrix = node->world().transform;
	auto originalPos = math::decomposePos(nodeMatrix);
	// auto i = glm::identity<glm::mat4>();
	// ImGuizmo::DrawGrid(glm::value_ptr(cameraView), glm::value_ptr(cameraProj), glm::value_ptr(i), 10.f);

	// TODO: Crappy way to simulate free drag in imguizmo, should probably implement internally
	static auto cameraView = camera.view;
	static auto cameraProj = camera.proj;
	if (!m_mouseLockedLastFrame) {
		cameraView = camera.view;
		cameraProj = camera.proj;
	}

	ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
		static_cast<ImGuizmo::OPERATION>(m_manipMode.op), static_cast<ImGuizmo::MODE>(m_manipMode.mode),
		glm::value_ptr(nodeMatrix));

	bool shouldMouseLock = false;
	if (ImGuizmo::IsUsing()) {
		if (!m_gizmoWasUsingLastFrame) {
			// ALT + DRAG duplicate
			if (Input.IsDown(Key::Alt)
				&& (m_manipMode.op == ManipOperationMode::Operation::Rotate
					|| m_manipMode.op == ManipOperationMode::Operation::Translate)) {
				ed::ClipboardOp::StoreEntity(node);
				auto loaded = ed::ClipboardOp::LoadEntity(node.GetWorld());
				OutlinerWindow::selected = loaded;
			}
		}
		if (Input.IsDown(Key::Shift)) {
			auto deltaPos = math::decomposePos(nodeMatrix) - originalPos;
			camera.MovePosition(deltaPos);
			shouldMouseLock = true;
		}
		node->SetNodeTransformWCS(nodeMatrix);
	}

	if (shouldMouseLock && !m_mouseLockedLastFrame) {
		Input.LockMouse();
	}
	else if (!shouldMouseLock && m_mouseLockedLastFrame) {
		Input.UnlockMouse();
	}
	m_mouseLockedLastFrame = shouldMouseLock;

	m_gizmoWasUsingLastFrame = ImGuizmo::IsUsing();
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
