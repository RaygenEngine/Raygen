#include "pch.h"
#include "EdOutlinerWindow.h"

#include "editor/DataStrings.h"
#include "editor/Editor.h"
#include "universe/NodeFactory.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/RootNode.h"
#include "universe/Universe.h"
#include "universe/WorldOperationsUtl.h"

#include <imgui/imgui.h>
namespace ed {
void OutlinerWindow::ImguiDraw()
{

	ImGui::BeginChild(
		"Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Spacing();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 6.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 6.f));

	bool foundOpen = false;
	RecurseNodes(Universe::MainWorld->GetRoot(), [&](Node* node, int32 depth) {
		auto str = std::string(depth * 6, ' ') + U8(node->GetClass().GetIcon()) + "  "
				   + sceneconv::FilterNodeClassName(node->GetClass().GetName()) + "> " + node->GetName();
		ImGui::PushID(node);

		auto editorCam = Editor::EditorInst->m_editorCamera;


		if (node == editorCam) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.58f, 0.58f, 0.95f));
			// ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.5f, 0.0f, 0.0f, 0.95f));
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.28f, 0.01f, 0.10f, 0.95f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.3f, 0.02f, 0.09f, 0.95f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.36f, 0.04f, 0.11f, 0.95f));
		}

		if (ImGui::Selectable(str.c_str(), node == EditorObject::GetSelectedNode())) {
			EditorObject::SelectNode(node);
		}

		if (node != editorCam) {
			Run_ContextPopup(node);
			Run_OutlinerDropTarget(node);
		}
		else {
			ImGui::PopStyleColor(4);

			if (ImGui::BeginPopupContextItem("OutlinerElemContext")) {
				if (!editorCam->GetParent()->IsRoot()) {
					if (ImGui::MenuItem("Stop piloting")) {
						worldop::MakeChildOf(Universe::MainWorld->GetRoot(), editorCam);
						editorCam->ResetRotation();
					}
				}
				if (ImGui::MenuItem("Set As Active")) {
					Universe::MainWorld->SetActiveCamera(editorCam);
				}
				ImGui::EndPopup();
			}
			else {
				ImEd::CollapsingHeaderHelpTooltip(help_EditorCamera);
			}
		}
		if (ImGui::IsPopupOpen("OutlinerElemContext")) {
			foundOpen = true;
		}
		ImGui::PopID();
	});

	ImGui::PopStyleVar(2);
	ImGui::EndChild();

	if (auto entry = ImEd::AcceptTypedPodDrop<ModelPod>()) {
		auto cmd = [&, entry]() {
			auto newNode = NodeFactory::NewNode<GeometryNode>();

			newNode->SetName(entry->GetNameStr());
			newNode->SetModel(entry->GetHandleAs<ModelPod>());
			Universe::MainWorld->Z_RegisterNode(newNode, Universe::MainWorld->GetRoot());
			if (!Editor::EditorInst->IsCameraPiloting()) {
				EditorObject::PushPostFrameCommand([newNode]() { EditorObject::FocusNode(newNode); });
			}
		};

		EditorObject::PushCommand(cmd);
	}


	if (ImGui::BeginDragDropTarget()) {
		std::string payloadTag = "POD_UID_" + std::to_string(refl::GetId<ModelPod>().hash());
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag.c_str())) {
			assert(payload->DataSize == sizeof(size_t));
			size_t uid = *reinterpret_cast<size_t*>(payload->Data);

			auto* podEntry = AssetHandlerManager::GetEntry(BasePodHandle{ uid });

			auto cmd = [&, uid, podEntry]() {
				auto newNode = NodeFactory::NewNode<GeometryNode>();

				newNode->SetName(podEntry->name);
				newNode->SetModel(PodHandle<ModelPod>{ uid });
				Universe::MainWorld->Z_RegisterNode(newNode, Universe::MainWorld->GetRoot());
				if (!Editor::EditorInst->IsCameraPiloting()) {
					EditorObject::PushPostFrameCommand([newNode]() { EditorObject::FocusNode(newNode); });
				}
			};

			EditorObject::PushCommand(cmd);
		}
		ImGui::EndDragDropTarget();
	}


	if (!foundOpen) {
		ImGui::PushID(989);
		if (ImGui::BeginPopupContextItem("RightclickOutliner Context")) {
			if (ImGui::BeginMenu("New Node")) {
				Run_NewNodeMenu(Universe::MainWorld->GetRoot());
				ImGui::EndMenu();
			}
			if (Editor::EditorInst->IsCameraPiloting() && ImGui::MenuItem("Stop piloting")) {
				EditorObject::PilotThis(nullptr);
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}


bool OutlinerWindow::Run_ContextPopup(Node* node)
{
	if (ImGui::BeginPopupContextItem("OutlinerElemContext")) {
		for (auto& action : Editor::EditorInst->m_nodeContextActions->GetActions(node, true)) {
			if (!action.IsSplitter()) {
				if (ImGui::MenuItem(action.name)) {
					action.function(node);
				}
			}
			else {
				ImGui::Separator();
			}
		}
		ImGui::Separator();

		if (ImGui::BeginMenu("Add Child")) {
			Run_NewNodeMenu(node);
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
		return true;
	}
	return false;
}

void OutlinerWindow::Run_NewNodeMenu(Node* underNode)
{
	auto factory = Universe::MainWorld->GetNodeFactory();


	for (auto& entry : factory->Z_GetEntries()) {
		if (ImGui::MenuItem(entry.first.c_str())) {

			auto cmd = [underNode, entry]() {
				auto newNode = entry.second.newInstance();

				newNode->SetName(entry.first + "_new");
				Universe::MainWorld->Z_RegisterNode(newNode, underNode);

				DirtyFlagset temp;
				temp.set();

				newNode->SetDirtyMultiple(temp);
			};

			EditorObject::PushCommand(cmd);
		}
	}
}

void OutlinerWindow::Run_OutlinerDropTarget(Node* node)
{
	// Drag Source
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
		ImGui::SetDragDropPayload("WORLD_REORDER", &node, sizeof(Node**));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WORLD_REORDER")) {
			CLOG_ABORT(payload->DataSize != sizeof(Node**), "Incorrect drop operation.");
			Node** dropSource = reinterpret_cast<Node**>(payload->Data);
			worldop::MakeChildOf(node, *dropSource);
		}
		ImGui::EndDragDropTarget();
	}
}

} // namespace ed
