#include "EdOutlinerWindow.h"

#include "assets/pods/Prefab.h"
#include "universe/ComponentsDb.h"
#include "editor/EditorObject.h"
#include "universe/Universe.h"


#include "editor/EdClipboardOp.h"
#include "editor/imgui/ImAssetSlot.h"
#include "editor/windows/general/EdAssetsWindow.h"


namespace ed {
namespace {
	Entity AddEntityMenu(World& world, const char* menuName)
	{
		Entity ent;
		if (ImGui::BeginMenu(menuName)) {
			if (auto entityType = ImEd::ComponentClassMenu(); entityType) {
				std::string name = entityType->clPtr->GetNameStr();
				name = name.substr(1) + " Entity";

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

void OutlinerWindow::DrawRecurseEntity(World& world, Entity ent, int32 depth)
{
	ImGui::PushID(static_cast<uint32>(ent.entity));

	ImGui::Selectable(U8(u8" " FA_EYE u8" "), false, 0, ImVec2(18.f, 0));
	ImGui::SameLine();
	ImGui::Indent(16.f * depth + 30.0f);
	if (m_renameFrame >= 0 && ent == OutlinerWindow::selected) {

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
		if (ImGui::Selectable(ent->name.c_str(), ent == OutlinerWindow::selected)) {
			OutlinerWindow::selected = ent;
		}
	}
	ImGui::Unindent(16.f * depth + 30.0f);

	if (ImGui::BeginPopupContextItem()) {
		OutlinerWindow::selected = ent;
		Run_ContextPopup(world, ent);
		ImGui::EndPopup();
	}
	Run_OutlinerDropEntity(ent);

	auto child = ent->firstChild;
	while (child) {
		DrawRecurseEntity(world, child, depth + 1);
		child = child->next;
	}

	ImGui::PopID();
}


void OutlinerWindow::ImguiDraw()
{
	auto& world = *Universe::MainWorld;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 6.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 6.f));


	for (auto& [ent, bs] : world.reg.view<BasicComponent>().each()) {
		if (!bs.parent) {
			DrawRecurseEntity(world, { ent, &world.reg });
		}
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
		OutlinerWindow::selected = {};
	}


	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiMouseButton_Right, false)) {
		Run_SpaceContextPopup(world);
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(2);

	if (postIterCommand) {
		postIterCommand();
		postIterCommand = {};
	}
}

void OutlinerWindow::Run_ContextPopup(World& world, Entity entity)
{
	if (Entity ent = AddEntityMenu(world, ETXT(FA_USER_PLUS, " Add Child Entity")); ent) {
		ent->SetParent(entity);
		ImGui::CloseCurrentPopup();
	}

	// TODO: ECS hide components already in entity
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
		entity.Destroy();
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
		entity.Destroy();
	}

	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_CERTIFICATE, " Make Prefab"))) {
		std::string name = "gen-data/Prefab " + entity->name;
		auto [entry, ptr] = AssetRegistry::CreateEntry<Prefab>(name);
		ptr->MakeFrom(world.reg, entity.entity);
		ed::AssetsWindow::RefreshEntries();
	}
	ImGui::Separator();
	if (ImGui::MenuItem(ETXT(FA_EMPTY, " Focus"), "F")) {
		EditorObject->edCamera.Focus(entity);
	}
	if (ImGui::MenuItem(ETXT(FA_EMPTY, " Teleport To Camera"))) {
		EditorObject->edCamera.TeleportToCamera(entity);
	}
	if (ImGui::MenuItem(ETXT(FA_PLANE, " Pilot"), "Shift+F")) {
		EditorObject->edCamera.Pilot(entity);
	}
}

void OutlinerWindow::Run_SpaceContextPopup(World& world)
{
	if (Entity ent = AddEntityMenu(world, "Add Entity"); ent) {
		ImGui::CloseCurrentPopup();
	}
	if (ImGui::MenuItem(ETXT(FA_PASTE, " Paste"), "Ctrl+V")) {
		ed::ClipboardOp::LoadEntity(world.reg);
	}


	if (ImGui::BeginMenu(ETXT(FA_BOXES, " From Prefab"))) {
		PodHandle<Prefab> prefab;
		if (ImEd::AssetSlot("", prefab)) {
			prefab.Lock()->InsertInto(world.reg);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndMenu();
	}
}

void OutlinerWindow::Run_OutlinerDropEntity(Entity entity)
{
	if (ImGui::BeginDragDropSource()) {
		ImEd::EndDragDropSourceByCopy(entity, "WORLD_ENTITY");
	}

	if (auto opt = ImEd::AcceptDropByCopy<Entity>("WORLD_ENTITY"); opt) {
		Entity dropEntity = *opt;
		if (dropEntity != entity) {
			postIterCommand = [=]() mutable {
				dropEntity->SetParent(entity);
			};
		}
	}
}

} // namespace ed
