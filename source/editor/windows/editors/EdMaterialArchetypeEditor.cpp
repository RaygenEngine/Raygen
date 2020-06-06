#include "pch.h"
#include "EdMaterialArchetypeEditor.h"
#include "assets/PodEditor.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImAssetSlot.h"
#include "assets/pods/Material.h"
#include "assets/AssetRegistry.h"
#include "assets/util/SpirvReflector.h"
#include "assets/util/SpirvCompiler.h"

#include "editor/windows/general/EdPropertyEditorWindow.h"

#include <imguicolortextedit/TextEditor.h>

namespace ed {
MaterialArchetypeEditorWindow::MaterialArchetypeEditorWindow(PodEntry* inEntry)
	: AssetEditorWindowTemplate(inEntry)
{

	editor.reset(new GenericShaderEditor(
		podHandle.Lock()->code, inEntry->path, [&]() { OnSave(); }, [&]() { OnCompile(); }));
}

void MaterialArchetypeEditorWindow::ImguiDraw()
{
	auto& uboMembers = podHandle.Lock()->parameters.uboMembers;
	auto& samplers = podHandle.Lock()->parameters.samplers2d;


	ImGui::Text(U8(FA_DATABASE u8" Descr Data (%d,%d) | "), samplers.size(), uboMembers.size());
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::SetWindowFontScale(1);
		ImGui::Text("Detected samplers and ubo Members:");

		for (auto& sampler : samplers) {
			ImGui::TextUnformatted(sampler.c_str());
		}

		for (auto& mem : uboMembers) {
			ImGui::Text(
				"%d\t%s\t%s", mem.SizeOf(), std::string(GenMetaEnum(mem.type).GetValueStr()).c_str(), mem.name.c_str());
		}
		ImGui::EndTooltip();
	}
	ImGui::SameLine();
	editor->ImguiDraw();
}

void MaterialArchetypeEditorWindow::OnSave()
{
	OnCompile();
	SaveToDisk();
}

void MaterialArchetypeEditorWindow::OnCompile()
{
	PodEditor ed(podHandle);

	ed.GetUpdateInfoRef().AddFlag("editor");

	ed->code = editor->editor->GetText();

	TextCompilerErrors errors;
	auto result = ShaderCompiler::Compile(ed->code, ShaderStageType::Fragment, &errors);
	if (result.size() > 0) {
		ed->binary.swap(result);
		ed->OnShaderUpdated();
	}
	editor->editor->SetErrorMarkers(errors.errors);
}


void MaterialInstanceEditorWindow::ImguiDraw()
{
	OptionalPodEditor ed(podHandle);
	auto material = ed.BeginOptionalEditRegion();

	auto archetype = material->archetype.Lock();
	auto archetypeEntry = AssetHandlerManager::GetEntry(material->archetype);

	if (ImEd::AssetSlot("Archetype", material->archetype)) {
		ed.MarkEdit();

		PodEditor arch(material->archetype);
		arch->instances.push_back(podHandle);
		arch->OnShaderUpdated();
	}

	ImGui::Separator();

	RuntimeClass* classDescription = material->archetype.Lock()->classDescr.get();
	if (!classDescription) {
		ImGui::Text("Generate the class of the archetype first.");
		if (ImEd::Button("GENERATE")) {
			PodEditor archetypeEditor(material->archetype);
			archetypeEditor.pod->OnShaderUpdated();
		}
		return;
	}


	int32 i = 0;
	for (auto& img : archetype->parameters.samplers2d) {
		if (ImEd::AssetSlot(img.c_str(), material->samplers2d[i])) {
			ed.MarkEdit();
		}
		++i;
	}

	if (GenericImguiDrawClass(material->uboData.data(), *classDescription)) {
		ed.MarkEdit();
	}
}

} // namespace ed
