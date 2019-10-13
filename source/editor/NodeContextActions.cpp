#include "pch/pch.h"

#include "NodeContextActions.h"
#include "system/Engine.h"
#include "editor/Editor.h"
#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"
#include <imgui/imgui.h>


NodeContextActions::NodeContextActions()
{
	baseActions.emplace_back("Duplicate", &Editor::Duplicate);
	baseActions.emplace_back("Delete", &Editor::Delete);
	baseActions.emplace_back();

	baseActions.emplace_back("Up", &Editor::MoveChildUp);
	baseActions.emplace_back("Down", &Editor::MoveChildDown);
	baseActions.emplace_back("Out", &Editor::MoveChildOut);
	baseActions.emplace_back("Move Selected Under", &Editor::MoveSelectedUnder);

	baseActions.emplace_back();
	baseActions.emplace_back("Look Through", &Editor::LookThroughThis);
	baseActions.emplace_back("Teleport to Camera", &Editor::TeleportToCamera);
}

std::vector<NodeContextActions::Entry> NodeContextActions::GetActions(Node* node)
{
	if (node->IsRoot()) {
		std::vector<Entry> actions;
		actions.emplace_back("Move Selected Under", &Editor::MoveSelectedUnder);
		return std::move(actions);
	}

	std::vector<Entry> actions = baseActions;


	if (node->IsA<CameraNode>()) {
		actions.emplace_back();
		actions.emplace_back("Set As Active", &Editor::MakeActiveCamera);
	}

	return std::move(actions);
}
