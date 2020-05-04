#include "pch.h"
#include "EdShaderStageEditorWindow.h"

#include "assets/AssetRegistry.h"
#include "assets/pods/Shader.h"
#include "assets/pods/ShaderStage.h"
#include "engine/Input.h"
#include "editor/imgui/ImEd.h"
#include "assets/util/SpirvCompiler.h"
#include "assets/PodEditor.h"

#include <imguicolortextedit/TextEditor.h>


ed::ShaderStageEditorWindow::ShaderStageEditorWindow(PodEntry* inEntry)
	: AssetEditorWindowTemplate(inEntry)
{
	editor = std::make_unique<TextEditor>();
	editor->SetColorizerEnable(true);
	editor->SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	editor->SetShowWhitespaces(false);

	editor->SetText(entry->GetHandleAs<ShaderStage>().Lock()->code);

	filepathCache = entry->metadata.originalImportLocation;
	filepathCache = fs::relative(filepathCache).generic_string();
}

void ed::ShaderStageEditorWindow::ImguiDraw()
{
	ImGui::Text(filepathCache.c_str());
	if (needsSave) {
		ImGui::SameLine();
		if (ImEd::Button(ETXT(FA_SAVE, "Save"))) {
			SaveInternal();
		}
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(83)) {
			SaveInternal();
		}
	}
	if (entry->metadata.originalImportLocation.empty()) {
		OptionalPodEditor<ShaderStage> ed(podHandle);

		if (ImEd::EnumDropDown("Stage", ed.BeginOptionalEditRegion()->stage)) {
			ed.CommitForGpu();
		}
	}
	ImEd::BeginCodeFont();
	editor->Render(filepathCache.c_str());

	if (editor->IsTextChanged()) {
		needsSave = true;
		SaveInternal();
	}
	ImEd::EndCodeFont();
}

void ed::ShaderStageEditorWindow::SaveInternal()
{
	needsSave = false;
	{

		PodEditor podeditor(entry->GetHandleAs<ShaderStage>());
		auto pod = podeditor.GetEditablePtr();

		pod->code = editor->GetText();


		TextCompilerErrors errors;
		if (entry->metadata.originalImportLocation.empty()) {
			pod->binary = ShaderCompiler::Compile(pod->code, pod->stage, entry->name, &errors);
		}
		else {
			pod->binary = ShaderCompiler::Compile(pod->code, std::string(uri::GetFilename(filepathCache)), &errors);
		}

		editor->SetErrorMarkers(errors.errors);
	}
	// SaveToDisk();
}
