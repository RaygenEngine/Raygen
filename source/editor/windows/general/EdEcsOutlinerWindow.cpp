#include "pch.h"
#include "EdEcsOutlinerWindow.h"

#include "assets/pods/Mesh.h"
#include "assets/util/ParsingUtl.h"
#include "editor/DataStrings.h"
#include "editor/EditorObject.h"
#include "universe/NodeFactory.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/RootNode.h"
#include "universe/Universe.h"
#include "universe/WorldOperationsUtl.h"

#include <imgui/imgui.h>

namespace ed {
namespace {
	void DrawRecurseEntity(Entity ent, int32 depth = 0)
	{
		ImGui::PushID(static_cast<uint32>(ent.m_entity));

		auto str = std::string(depth * 6, ' ') + ent->name.c_str();

		if (ImGui::Selectable(str.c_str(), ent == EcsOutlinerWindow::selected)) {
			EcsOutlinerWindow::selected = ent;
		}
		auto child = ent->firstChild;
		while (child) {
			DrawRecurseEntity(child, depth + 1);
			child = child->next;
		}

		ImGui::PopID();
	}
} // namespace

void EcsOutlinerWindow::ImguiDraw()
{
	auto& world = *Universe::ecsWorld;
	if (ImEd::Button("Create")) {
		world.CreateEntity("New Entity");
	}
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 6.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 6.f));


	for (auto& [ent, bs] : world.reg.view<BasicComponent>().each()) {
		if (!bs.parent) {
			DrawRecurseEntity({ ent, &world.reg });
		}
	}
	ImGui::PopStyleVar(2);
}


// bool OutlinerWindow::Run_ContextPopup(Node* node)
//{
//	if (ImGui::BeginPopupContextItem("OutlinerElemContext")) {
//		for (auto& action : EditorObject->m_nodeContextActions->GetActions(node, true)) {
//			if (!action.IsSplitter()) {
//				if (ImGui::MenuItem(action.name)) {
//					action.function(node);
//				}
//			}
//			else {
//				ImGui::Separator();
//			}
//		}
//		ImGui::Separator();
//
//		if (ImGui::BeginMenu("Add Child")) {
//			Run_NewNodeMenu(node);
//			ImGui::EndMenu();
//		}
//
//		ImGui::EndPopup();
//		return true;
//	}
//	return false;
//}
//
// void OutlinerWindow::Run_OutlinerDropTarget(Node* node)
//{
//	// Drag Source
//	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
//		ImGui::SetDragDropPayload("WORLD_REORDER", &node, sizeof(Node**));
//		ImGui::EndDragDropSource();
//	}
//	if (ImGui::BeginDragDropTarget()) {
//		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WORLD_REORDER")) {
//			CLOG_ABORT(payload->DataSize != sizeof(Node**), "Incorrect drop operation.");
//			Node** dropSource = reinterpret_cast<Node**>(payload->Data);
//			worldop::MakeChildOf(node, *dropSource);
//		}
//		ImGui::EndDragDropTarget();
//	}
//}

} // namespace ed
