#pragma once
#include "editor/EditorObject.h"
#include "editor/imgui/ImGuizmo.h"
#include "editor/windows/EdWindow.h"
#include "reflection/Property.h"


namespace ed {

bool GenericImguiDrawClass(void* object, const ReflClass& cl);

struct ManipOperationMode {
	enum class Operation
	{
		Translate = ImGuizmo::TRANSLATE,
		Rotate = ImGuizmo::ROTATE,
		Scale = ImGuizmo::SCALE,
		Bounds = ImGuizmo::BOUNDS,
	};
	enum class Space
	{
		Local = ImGuizmo::LOCAL,
		World = ImGuizmo::WORLD,
	};

	Operation op;
	Space mode;
};

class PropertyEditorWindow : public UniqueWindow {
public:
	PropertyEditorWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void ImguiDraw();
	virtual ~PropertyEditorWindow() = default;

	bool m_localMode{ true };
	bool m_displayMatrix{ false };
	bool m_lockedScale{ false };

	bool m_lookAtMode{ false };

	glm::vec3 m_lookAtPos{ 0.f, 0.f, 0.f };

	ed::ManipOperationMode m_manipMode{};

	// Injects the imgui code of a prope

	void Run_BaseProperties(Entity ent);


	void Run_Components(Entity entity);

	void Run_ImGuizmo(Entity ent);

private:
	bool m_gizmoWasUsingLastFrame{};
	bool m_mouseLockedLastFrame{};
};
} // namespace ed
