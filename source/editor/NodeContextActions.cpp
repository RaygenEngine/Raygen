#include "pch/pch.h"

#include "NodeContextActions.h"
#include "system/Engine.h"
#include "editor/Editor.h"
#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"
#include <imgui.h>


NodeContextActions::NodeContextActions()
{
	baseActions.emplace_back("Duplicate", &Editor::Duplicate);
	baseActions.emplace_back("Delete", &Editor::Delete);
	baseActions.emplace_back();

	baseActions.emplace_back("Up", &Editor::MoveChildUp);
	baseActions.emplace_back("Down", &Editor::MoveChildDown);
	baseActions.emplace_back("Out", &Editor::MoveChildOut);
}

std::vector<NodeContextActions::Entry> NodeContextActions::GetActions(Node* node, bool extendedList)
{
	if (node->IsRoot()) {
		std::vector<Entry> actions;
		actions.emplace_back("Move Selected Under", &Editor::MoveSelectedUnder);
		return std::move(actions);
	}

	if (node->IsA<EditorCameraNode>()) {
		std::vector<Entry> actions;
		return std::move(actions);
	}

	std::vector<Entry> actions = baseActions;

	if (extendedList) {
		actions.emplace_back("Move Selected Under", &Editor::MoveSelectedUnder);

		actions.emplace_back();
		actions.emplace_back("Teleport to Camera", &Editor::TeleportToCamera);

		auto editorCam = Engine::GetEditor()->m_editorCamera;
		if (editorCam && editorCam->GetParent() == node) {
			actions.emplace_back("Stop Piloting", &Editor::PilotThis);
		}
		else {
			actions.emplace_back("Focus", &Editor::FocusNode);
			actions.emplace_back("Pilot", &Editor::PilotThis);
		}

		if (node->IsA<CameraNode>()) {
			actions.emplace_back();
			actions.emplace_back("Set As Active", &Editor::MakeActiveCamera);
		}
	}

	return std::move(actions);
}
