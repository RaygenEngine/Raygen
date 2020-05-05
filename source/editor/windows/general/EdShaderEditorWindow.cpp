#include "pch.h"
#include "EdShaderEditorWindow.h"


#include "assets/AssetRegistry.h"
#include "assets/pods/Shader.h"
#include "engine/Input.h"
#include "core/iterable/IterableSafeVector.h"
#include "editor/imgui/ImEd.h"
#include "assets/importers/ShaderImporter.h"
#include "rendering/assets/GpuAssetManager.h"

#include <imguicolortextedit/TextEditor.h>
#include <fstream>

namespace ed {
/*
void ShaderEditorWindow::OpenShaderForEditing(PodEntry* entry, bool isFrag)
{
	if (!entry->IsA<Shader>()) {
		LOG_ERROR("Attempting to open a non shader entry for shader editing.");
		return;
	}

	if (!FindEntry(entry, isFrag)) {
		documentWindows.Emplace(std::make_unique<ShaderDocumentEditor>(entry, isFrag));
	}
} // namespace ed

void ShaderEditorWindow::OnDraw(const char* title, bool* keepOpen)
{
	if (ImGui::Begin(title, keepOpen)) {
		for (auto& entry : AssetHandlerManager::Z_GetPods()) {
			if (entry->IsA<Shader>() && !entry->name.starts_with('~')) {
				std::string fragName = entry->name + ".frag";
				std::string vertName = entry->name + ".vert";


				bool f = false;
				if (ImGui::Selectable(fragName.c_str(), f)) {
					OpenShaderForEditing(entry.get(), true);
				}
				if (ImGui::Selectable(vertName.c_str(), f)) {
					OpenShaderForEditing(entry.get(), false);
				}
				ImEd::CreateTypedPodDrag(entry.get());
			}
		}
	}
	ImGui::End();

	documentWindows.BeginSafeRegion();
	for (auto& docWin : documentWindows.vec) {
		docWin->Draw();
	}
	documentWindows.EndSafeRegion();
}

*/
} // namespace ed
