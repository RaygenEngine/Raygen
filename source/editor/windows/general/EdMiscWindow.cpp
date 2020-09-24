#include "pch.h"
#include "EdMiscWindow.h"

#include "editor/DataStrings.h"
#include "editor/EditorObject.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiUtil.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneSpotlight.h"
#include "engine/Events.h"

#include <imgui/imgui.h>

namespace ed {
void HelpWindow::OnDraw(const char* title, bool* open)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
	ImGui::SetNextWindowPos(ImVec2(650.f, 100.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 720.f), ImGuiCond_Always);

	if (ImGui::Begin(title, open)) {
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 33.0f);
		ImGui::Text(txt_help);
		ImGui::Text("");
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void AboutWindow::ImguiDraw()
{
	ImGui::Text("Raygen: v0.1");
	ImEd::HSpace(220.f);
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 33.0f);
	ImGui::Text(txt_about);
	ImGui::Text("");
}

void ImGuiDemoWindow::OnDraw(const char* title, bool* open)
{
	ImGui::ShowDemoWindow(open);
}

void PodEntryEditorWindow::ImguiDraw()
{
	ImEd::Button("Drop an asset here.");
	ImEd::AcceptGenericPodDrop([&](auto, PodEntry* newEntry) { entry = newEntry; });

	if (entry) {
		ImGui::Checkbox("Reimport On Load", &entry->metadata.reimportOnLoad);
		ImGui::Checkbox("Export On Save", &entry->metadata.exportOnSave);
		ImGui::Checkbox("Transient", &entry->transient);
		ImGui::InputText("Import Path", &entry->metadata.originalImportLocation);
		ImGui::Text("Hash: %d", &entry->metadata.podTypeHash);
		ImGui::Text("===", &entry->metadata.podTypeHash);

		if (ImGui::Button("Mark Save")) {
			entry->MarkSave();
		}

		if (ImGui::Button("Reload From Disk")) {
			AssetHandlerManager::ReimportFromOriginal(entry);
		}
	}
}
} // namespace ed
