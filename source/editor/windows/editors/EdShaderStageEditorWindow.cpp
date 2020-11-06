#include "EdShaderStageEditorWindow.h"

#include "assets/PodEditor.h"
#include "assets/pods/ShaderStage.h"
#include "assets/util/SpirvCompiler.h"
#include "editor/imgui/ImEd.h"

#include <imguicolortextedit/TextEditor.h>

namespace ed {

GenericShaderEditor::GenericShaderEditor(const std::string& initialCode, const std::string& filepath,
	std::function<void()>&& onSaveCb, std::function<void()> onUpdateCb)
{
	editor = std::make_unique<TextEditor>();
	editor->SetColorizerEnable(true);
	editor->SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	editor->SetShowWhitespaces(false);

	editor->SetText(initialCode);
	filepathTitle = filepath;

	onSave = std::move(onSaveCb);
	onUpdate = std::move(onUpdateCb);
}

void GenericShaderEditor::ImguiDraw()
{
	ImGui::Text(filepathTitle.c_str());

	ImGui::SameLine();
	ImGui::Checkbox("Live Updates", &liveUpdates);
	if (needsSave) {
		ImGui::SameLine();
		if (ImEd::Button(ETXT(FA_SAVE, "Save")) || (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(83))) {
			needsSave = false;
			onSave();
		}
	}
	ImEd::BeginCodeFont();
	editor->Render(filepathTitle.c_str());

	if (editor->IsTextChanged()) {
		needsSave = true;
		if (liveUpdates) {
			onUpdate();
		}
	}
	ImEd::EndCodeFont();
}

} // namespace ed


ed::ShaderStageEditorWindow::ShaderStageEditorWindow(PodEntry* inEntry)
	: AssetEditorWindowTemplate(inEntry)
{
	filepathCache = fs::relative(entry->metadata.originalImportLocation).generic_string();

	editor.reset(new GenericShaderEditor(
		entry->GetHandleAs<ShaderStage>().Lock()->code,
		filepathCache,             //
		[&]() { SaveInternal(); }, //
		[&]() { UpdateForGpu(); }));
}

void ed::ShaderStageEditorWindow::ImguiDraw()
{
	if (entry->metadata.originalImportLocation.empty()) {
		OptionalPodEditor<ShaderStage> ed(podHandle);
		if (ImEd::EnumDropDown("Stage", ed.BeginOptionalEditRegion()->stage)) {
			ed.CommitForGpu();
			editor->needsSave = true;
		}
		ImGui::SameLine();
	}

	editor->ImguiDraw();
}

void ed::ShaderStageEditorWindow::UpdateForGpu()
{
	PodEditor pod(podHandle);

	pod.GetUpdateInfoRef().AddFlag("editor");

	pod->code = editor->editor->GetText();


	TextCompilerErrors errors;
	if (entry->metadata.originalImportLocation.empty()) {
		pod->binary = ShaderCompiler::Compile(pod->code, pod->stage, entry->name, &errors);
	}
	else {
		pod->binary = ShaderCompiler::Compile(pod->code, std::string(uri::GetFilename(filepathCache)), &errors);
	}

	editor->editor->SetErrorMarkers(errors.errors);
}

void ed::ShaderStageEditorWindow::SaveInternal()
{
	UpdateForGpu();
	SaveToDisk();
}
