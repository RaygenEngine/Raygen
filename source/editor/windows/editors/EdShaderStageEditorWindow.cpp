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

	if (entry->metadata.originalImportLocation.empty()) {
		OptionalPodEditor<ShaderStage> ed(podHandle);
		ImGui::SameLine();
		if (ImEd::EnumDropDown("Stage", ed.BeginOptionalEditRegion()->stage)) {
			ed.CommitForGpu();
			needsSave = true;
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Live Updates", &liveUpdates);
	if (needsSave) {
		ImGui::SameLine();
		if (ImEd::Button(ETXT(FA_SAVE, "Save"))) {
			SaveInternal();
		}
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(83)) {
			SaveInternal();
		}
	}
	ImEd::BeginCodeFont();
	editor->Render(filepathCache.c_str());

	if (editor->IsTextChanged()) {
		needsSave = true;
		if (liveUpdates) {
			UpdateForGpu();
		}
	}
	ImEd::EndCodeFont();
}

void ed::ShaderStageEditorWindow::UpdateForGpu()
{
	PodEditor podeditor(entry->GetHandleAs<ShaderStage>());
	auto pod = podeditor.GetEditablePtr();

	podeditor.GetUpdateInfoRef().AddFlag("editor");

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

void ed::ShaderStageEditorWindow::SaveInternal()
{
	UpdateForGpu();
	needsSave = false;
	SaveToDisk();
}
