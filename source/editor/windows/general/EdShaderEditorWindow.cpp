#include "pch.h"
#include "EdShaderEditorWindow.h"


#include "assets/AssetRegistry.h"
#include "assets/pods/ShaderPod.h"
#include "engine/Input.h"
#include "core/iterable/IterableSafeVector.h"
#include "editor/imgui/ImEd.h"
#include "assets/importers/ShaderImporter.h"
#include "rendering/asset/GpuAssetManager.h"

#include <imguicolortextedit/TextEditor.h>
#include <fstream>

namespace ed {
struct ShaderDocumentEditor;

IterableSafeVector<UniquePtr<ShaderDocumentEditor>> documentWindows;

struct ShaderDocumentEditor {
	PodEntry* entry;

	fs::path filepath;
	bool isFragment{ false };
	TextEditor editor;

	std::string windowName;
	std::string filepathCache;

	bool needsSave{ false };

	ShaderDocumentEditor(PodEntry* entry, bool isFragment)
		: entry(entry)
		, isFragment(isFragment)
	{
		editor.SetColorizerEnable(true);
		editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
		editor.SetShowWhitespaces(false);

		windowName = entry->name;
		if (isFragment) {
			windowName += ".frag";
			editor.SetText(entry->GetHandleAs<Shader>().Lock()->frag.code);
		}
		else {
			windowName += ".vert";
			editor.SetText(entry->GetHandleAs<Shader>().Lock()->vert.code);
		}

		windowName += "##";
		windowName += entry->metadata.originalImportLocation;


		filepath = entry->metadata.originalImportLocation;
		filepath.replace_extension(isFragment ? ".frag" : ".vert");

		filepathCache = filepath.generic_string();
	}

	void Draw()
	{
		bool open = true;
		if (ImGui::Begin(windowName.c_str(), &open)) {
			ImGui::Text(filepathCache.c_str());
			if (needsSave) {
				ImGui::SameLine();
				if (ImEd::Button(ETXT(FA_SAVE, "Save"))) {
					Save();
				}
				if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(83)) {
					Save();
				}
			}
			ImEd::BeginCodeFont();
			editor.Render(windowName.c_str());

			if (editor.IsTextChanged()) {
				needsSave = true;
			}
			ImEd::EndCodeFont();
		}
		if (!open) {
			auto it = std::find_if(documentWindows.vec.begin(), documentWindows.vec.end(),
				[&](const auto& ptr) { return ptr.get() == this; });
			documentWindows.Remove(it);
		}
		ImGui::End();
	}

	void Save()
	{
		needsSave = false;

		std::ofstream out(filepathCache);
		out << editor.GetText();
		out.close();

		auto& stage = isFragment ? entry->UnsafeGet<Shader>()->frag : entry->UnsafeGet<Shader>()->vert;
		TextCompilerErrors errors;
		stage = shd::LoadAndCompileStage(filepathCache, "", &errors); // WIP: Hack params

		editor.SetErrorMarkers(errors.errors);
		vl::GpuAssetManager->ShaderChanged(PodHandle<Shader>{ entry->uid });
	}
};

bool FindEntry(PodEntry* entry, bool isFrag)
{
	auto it = std::find_if(documentWindows.vec.begin(), documentWindows.vec.end(),
		[&](const auto& ptr) { return ptr->entry == entry && ptr->isFragment == isFrag; });
	return it != documentWindows.vec.end();
}


ShaderEditorWindow::ShaderEditorWindow(std::string_view name)
	: UniqueWindow(name)
{
}


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


} // namespace ed
