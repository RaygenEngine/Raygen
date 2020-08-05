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

#include "editor/EdMenu.h"

#include <imgui/imgui.h>


namespace ed {
namespace {
	Entity AddEntityMenu(ECS_World& world, const char* menuName)
	{
		Entity ent;
		if (ImGui::BeginMenu(menuName)) {
			for (auto& [name, type] : ComponentsDb::Z_GetNameToTypes()) {
				if (ImGui::Selectable(name.c_str())) {
					std::string entityName = name.substr(0, name.length() - 4) + " Entity";

					ent = world.CreateEntity(entityName);
					ComponentsDb::GetType(type)->emplace(*ent.m_registry, ent.m_entity);
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::IsItemClicked()) {
			ent = world.CreateEntity();
		}
		return ent;
	}
} // namespace

void EcsOutlinerWindow::DrawRecurseEntity(ECS_World& world, Entity ent, int32 depth)
{
	ImGui::PushID(static_cast<uint32>(ent.m_entity));

	auto str = std::string(depth * 6, ' ') + ent->name.c_str();


	if (ImGui::Selectable(str.c_str(), ent == EcsOutlinerWindow::selected)) {
		EcsOutlinerWindow::selected = ent;
	}

	if (ImGui::BeginPopupContextItem()) {
		EcsOutlinerWindow::selected = ent;
		Run_ContextPopup(world, ent);
		ImGui::EndPopup();
	}
	auto child = ent->firstChild;
	while (child) {
		DrawRecurseEntity(world, child, depth + 1);
		child = child->next;
	}

	ImGui::PopID();
}


void EcsOutlinerWindow::ImguiDraw()
{
	auto& world = *Universe::ecsWorld;


	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 6.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 6.f));


	for (auto& [ent, bs] : world.reg.view<BasicComponent>().each()) {
		if (!bs.parent) {
			DrawRecurseEntity(world, { ent, &world.reg });
		}
	}

	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiMouseButton_Right, false)) {
		if (AddEntityMenu(world, "Add Entity")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(2);
}

void EcsOutlinerWindow::Run_ContextPopup(ECS_World& world, Entity entity)
{
	if (AddEntityMenu(world, ETXT(FA_PLUS, " Add Child"))) {
		ImGui::CloseCurrentPopup();
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_CUT, " Cut"))) {
	}
	if (ImGui::MenuItem(ETXT(FA_COPY, " Copy"))) {
	}
	if (ImGui::MenuItem(ETXT(FA_PASTE, " Paste"))) {
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_COPY, " Duplicate"))) {
	}
	if (ImGui::MenuItem(ETXT(FA_TRASH, " Delete"))) {
	}
	if (ImGui::MenuItem(ETXT(FA_EDIT, " Rename"))) {
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_CERTIFICATE, " Make Prefab"))) {
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_EMPTY, " Focus"))) {
	}
	if (ImGui::MenuItem(ETXT(FA_EMPTY, " Teleport To Camera"))) {
	}
	if (ImGui::MenuItem(ETXT(FA_PLANE, " Pilot"))) {
	}
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
