#include "pch.h"
#include "NodeContextActions.h"

#include "editor/EditorObject.h"
#include "engine/Engine.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/Node.h"
#include "universe/WorldOperationsUtl.h"

#include <imgui.h>


NodeContextActions::NodeContextActions()
{
	baseActions.emplace_back("Duplicate", &EditorObject::Duplicate);
	baseActions.emplace_back("Delete", &EditorObject::Delete);
	baseActions.emplace_back();

	baseActions.emplace_back("Up", &worldop::MoveChildUp);
	baseActions.emplace_back("Down", &worldop::MoveChildDown);
	baseActions.emplace_back("Out", &worldop::MoveChildOut);
}

std::vector<NodeContextActions::Entry> NodeContextActions::GetActions(Node* node, bool extendedList)
{
	if (node->IsRoot()) {
		std::vector<Entry> actions;
		actions.emplace_back("Move Selected Under", &EditorObject::MoveSelectedUnder);
		return std::move(actions);
	}

	if (node->IsA<EditorCameraNode>()) {
		std::vector<Entry> actions;
		return std::move(actions);
	}

	std::vector<Entry> actions = baseActions;

	if (extendedList) {
		actions.emplace_back("Move Selected Under", &EditorObject::MoveSelectedUnder);

		actions.emplace_back();
		actions.emplace_back("Teleport to Camera", &EditorObject::TeleportToCamera);

		auto editorCam = EditorObj->m_editorCamera;
		if (editorCam && editorCam->GetParent() == node) {
			actions.emplace_back("Stop Piloting", &EditorObject::PilotThis);
		}
		else {
			actions.emplace_back("Focus", &EditorObject::FocusNode);
			actions.emplace_back("Pilot", &EditorObject::PilotThis);
		}

		if (node->IsA<CameraNode>()) {
			actions.emplace_back();
			actions.emplace_back("Set As Active", &worldop::MakeActiveCamera);
		}
	}

	return actions;
}
