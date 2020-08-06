#include "pch.h"
#include "EdEcsOutlinerWindow.h"

#include "assets/pods/Mesh.h"
#include "assets/util/ParsingUtl.h"
#include "ecs_universe/ComponentsDb.h"
#include "editor/DataStrings.h"
#include "editor/EditorObject.h"
#include "universe/NodeFactory.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/RootNode.h"
#include "universe/Universe.h"
#include "universe/WorldOperationsUtl.h"

#include "editor/EdMenu.h"
#include "editor/EdClipboardOp.h"

#include <imgui/imgui.h>


namespace ed {
namespace {
	Entity AddEntityMenu(ECS_World& world, const char* menuName)
	{
		Entity ent;
		if (ImGui::BeginMenu(menuName)) {
			if (auto entityType = ImEd::ComponentClassMenu(); entityType) {
				std::string name = entityType->clPtr->GetNameStr();
				name = name.substr(0, name.length() - 4) + " Entity";

				ent = world.CreateEntity(name);
				entityType->emplace(*ent.registry, ent.entity);
			}
			ImGui::EndMenu();
		}

		if (ImGui::IsItemClicked()) {
			ent = world.CreateEntity();
			ImGui::CloseCurrentPopup();
		}
		return ent;
	}
} // namespace
void EcsOutlinerWindow::DrawRecurseEntity(ECS_World& world, Entity ent, int32 depth)
{
	ImGui::PushID(static_cast<uint32>(ent.entity));


	// if (m_renameStatus != RenameStatus::Inactive && ent == EcsOutlinerWindow::selected) {
	//	if (m_renameStatus == RenameStatus::FirstFrame) {
	//		ImGui::SetKeyboardFocusHere();
	//		m_renameString = ent->name;
	//	}

	//	ImGui::SetNextItemWidth(-1.f);

	//	if (ImGui::InputText("###RenameString", &m_renameString, ImGuiInputTextFlags_EnterReturnsTrue)
	//		|| (!ImGui::IsItemFocused() && m_renameStatus == RenameStatus::OtherFrames)) {
	//		ent->name = m_renameString;
	//		m_renameStatus = RenameStatus::Inactive;
	//	}
	//	else {
	//		m_renameStatus = RenameStatus::OtherFrames;
	//	}
	//}
	// else {
	//	if (ImGui::Selectable(str.c_str(), ent == EcsOutlinerWindow::selected)) {
	//		EcsOutlinerWindow::selected = ent;
	//	}
	//}


	ImGui::Selectable(U8(u8" " FA_EYE u8" "), false, 0, ImVec2(18.f, 0));
	ImGui::SameLine();
	ImGui::Indent(16.f * depth + 30.0f);
	if (m_renameFrame >= 0 && ent == EcsOutlinerWindow::selected) {

		if (m_renameFrame == 0) {
			ImGui::SetKeyboardFocusHere();
			m_renameString = ent->name;
		}


		ImGui::SetNextItemWidth(-1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.f, 0.f));
		if (ImGui::InputText("###RenameString", &m_renameString)) {
			ent->name = m_renameString;
		}
		ImGui::PopStyleVar();
		m_renameFrame++;
		if (!ImGui::IsItemActive() && m_renameFrame > 5) {
			m_renameFrame = -1;
		}
	}
	else {
		if (ImGui::Selectable(ent->name.c_str(), ent == EcsOutlinerWindow::selected)) {
			EcsOutlinerWindow::selected = ent;
		}
	}
	ImGui::Unindent(16.f * depth + 30.0f);

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
} // namespace ed


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
		Run_SpaceContextPopup(world);
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(2);
}

void EcsOutlinerWindow::Run_ContextPopup(ECS_World& world, Entity entity)
{
	// WIP: ECS
	if (Entity ent = AddEntityMenu(world, ETXT(FA_USER_PLUS, " Add Child Entity")); ent) {
		ent->SetParent(entity);
		ImGui::CloseCurrentPopup();
	}
	if (ImGui::BeginMenu(ETXT(FA_PLUS, " Add Component"))) {
		if (auto compType = ImEd::ComponentClassMenu(); compType) {
			compType->emplace(*entity.registry, entity.entity);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndMenu();
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_EDIT, " Rename"), "F2")) {
		m_renameStatus = RenameStatus::FirstFrame;
		m_renameFrame = 0;
	}
	if (ImGui::MenuItem(ETXT(FA_CUT, " Cut"), "Ctrl+X")) {
		ed::ClipboardOp::StoreEntity(entity);
		world.DestroyEntity(entity);
	}
	if (ImGui::MenuItem(ETXT(FA_COPY, " Copy"), "Ctrl+C")) {
		ed::ClipboardOp::StoreEntity(entity);
	}
	if (ImGui::MenuItem(ETXT(FA_PASTE, " Paste"), "Ctrl+V")) {
		ed::ClipboardOp::LoadEntity(world.reg)->SetParent(entity);
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_COPY, " Duplicate"), "Ctrl+W")) {
		// TODO: ECS: Actual duplicate instead of copy paste
		ed::ClipboardOp::StoreEntity(entity);
		auto loaded = ed::ClipboardOp::LoadEntity(world.reg);
		loaded->SetParent(entity->parent);
	}
	if (ImGui::MenuItem(ETXT(FA_TRASH, " Delete"), "Del")) {
		world.DestroyEntity(entity);
	}

	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_CERTIFICATE, " Make Prefab"))) {
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_EMPTY, " Focus"), "F")) {
	}
	if (ImGui::MenuItem(ETXT(FA_EMPTY, " Teleport To Camera"))) {
	}
	if (ImGui::MenuItem(ETXT(FA_PLANE, " Pilot"), "Shift+F")) {
	}
}

void EcsOutlinerWindow::Run_SpaceContextPopup(ECS_World& world)
{
	if (Entity ent = AddEntityMenu(world, "Add Entity"); ent) {
		ImGui::CloseCurrentPopup();
	}
	if (ImGui::MenuItem(ETXT(FA_PASTE, " Paste"), "Ctrl+V")) {
		ed::ClipboardOp::LoadEntity(world.reg);
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
