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

namespace {
	std::stringstream GetUniformText(const DynamicDescriptorSetLayout& layout)
	{
		std::stringstream uboText;

		for (auto& prop : layout.uboClass.GetProperties()) {
			if (prop.IsA<glm::vec4>()) {
				if (prop.HasFlags(PropertyFlags::Color)) {
					uboText << "col4 ";
				}
				else {
					uboText << "vec4 ";
				}
			}
			else {
				uboText << prop.GetType().name() << " ";
			}
			uboText << prop.GetName() << ";\n";
		}
		uboText << "\n";

		uboText << "ubo " << layout.uboName << ";\n\n";

		for (auto& sampler : layout.samplers2d) {
			uboText << "sampler2D " << sampler << ";\n";
		}
		return uboText;
	}

	// Returns true if the "compilation" was successful
	bool ValidateUniforms(TextEditor& uniformEditor, DynamicDescriptorSetLayout& layout)
	{
		TextEditor::ErrorMarkers errors;

		std::unordered_set<std::string, str::Hash> identifiers;
		layout.samplers2d.clear();
		layout.uboClass.ResetProperties();

		for (uint32 lineNum = 0; auto& line : uniformEditor.GetTextLines()) {
			lineNum++;
			if (line.starts_with("//") || line.size() < 1) {
				continue;
			}

			auto parts = str::split(line, " ;");
			if (parts.size() < 2) {
				errors.emplace(lineNum, "Expected format for each line is: 'type name;'");
				continue;
			}


			if (*(parts[1].data() + parts[1].size()) != ';') {
				errors.emplace(lineNum, "Expected a ';'");
				continue;
			}

			// PERF: Do hashing on strview
			std::string id = std::string(parts[1]);
			if (identifiers.contains(id)) {
				errors.emplace(lineNum, fmt::format("Duplicate identifier: {}.", id));
				continue;
			}

			identifiers.insert(id);

			if (str::equal(parts[0], "vec4")) {
				layout.uboClass.AddProperty<glm::vec4>(id);
			}
			else if (str::equal(parts[0], "col4")) {
				layout.uboClass.AddProperty<glm::vec4>(id, PropertyFlags::Color);
			}
			else if (str::equal(parts[0], "int")) {
				layout.uboClass.AddProperty<int>(id);
			}
			else if (str::equal(parts[0], "float")) {
				layout.uboClass.AddProperty<float>(id);
			}
			else if (str::equal(parts[0], "ubo")) {
				layout.uboName = id;
			}
			else if (str::equal(parts[0], "sampler2d")) {
				layout.samplers2d.push_back(id);
			}
			else {
				errors.emplace(lineNum, "Unknown variable type.");
				continue;
			}
		}
		uniformEditor.SetErrorMarkers(errors);
		return errors.size() == 0;
	}
} // namespace


MaterialArchetypeEditorWindow::MaterialArchetypeEditorWindow(PodEntry* inEntry)
	: AssetEditorWindowTemplate(inEntry)
{

	editor.reset(new GenericShaderEditor(
		podHandle.Lock()->gbufferFragMain, inEntry->path, [&]() { OnSave(); }, [&]() { OnCompile(); }));

	uniformEditor.reset(new TextEditor());

	uniformEditor->SetText(GetUniformText(podHandle.Lock()->descriptorSetLayout).str());
	uniformEditor->SetColorizerEnable(true);
	uniformEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	uniformEditor->SetShowWhitespaces(false);
}

void MaterialArchetypeEditorWindow::ImguiDraw()
{
	if (ImGui::CollapsingHeader("Uniform Variables")) {
		ImEd::BeginCodeFont();
		uniformEditor->Render("UniformVariables", ImVec2(0, uniformEditor->GetTotalLines() * 18.f + 22.f));
		ImEd::EndCodeFont();
		if (uniformEditor->IsTextChanged()) {
			ValidateUniforms(*uniformEditor, editingDescSet);
		}
	}


	// auto& uboMembers = podHandle.Lock()->parameters.uboMembers;
	// auto& samplers = podHandle.Lock()->parameters.samplers2d;


	// ImGui::Text(U8(FA_DATABASE u8" Descr Data (%d,%d) | "), samplers.size(), uboMembers.size());
	// if (ImGui::IsItemHovered()) {
	//	ImGui::BeginTooltip();
	//	ImGui::SetWindowFontScale(1);
	//	ImGui::Text("Detected samplers and ubo Members:");

	//	for (auto& sampler : samplers) {
	//		ImGui::TextUnformatted(sampler.c_str());
	//	}

	//	for (auto& mem : uboMembers) {
	//		ImGui::Text(
	//			"%d\t%s\t%s", mem.SizeOf(), std::string(GenMetaEnum(mem.type).GetValueStr()).c_str(), mem.name.c_str());
	//	}
	//	ImGui::EndTooltip();
	//}
	// ImGui::SameLine();
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

	ed->gbufferFragMain = editor->editor->GetText();

	if (!ValidateUniforms(*uniformEditor, editingDescSet)) {
		editor->editor->SetErrorMarkers({ { 1, "Uniform Compilation Error." } });
		return;
	}

	TextCompilerErrors errors;
	bool result = ed->GenerateGBufferFrag(editingDescSet, &errors);

	if (result) {
		ed->ChangeLayout(std::move(editingDescSet));
		editingDescSet = {};
		editor->editor->SetErrorMarkers({});
		return;
	}

	std::map<int, std::string> additionalErrors;
	for (auto& err : errors.errors) {
		if (err.first >= 100000) {
			additionalErrors.insert({ additionalErrors.size() + 1, err.second });
		}
	}

	if (additionalErrors.size() > 0) {
		editor->editor->SetErrorMarkers(additionalErrors);
	}
	else {
		editor->editor->SetErrorMarkers(errors.errors);
	}
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
		if (auto it = std::find(arch->instances.begin(), arch->instances.end(), podHandle);
			it == arch->instances.end()) {
			arch->instances.push_back(podHandle);
		}
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Separator();

	const RuntimeClass& classDescription = material->archetype.Lock()->descriptorSetLayout.uboClass;

	if (material->descriptorSet.uboData.size() != classDescription.GetSize()) {
		ImGui::Text("Incorrect uboData size!");
		return;
	}

	int32 i = 0;
	for (auto& img : archetype->descriptorSetLayout.samplers2d) {
		if (ImEd::AssetSlot(img.c_str(), material->descriptorSet.samplers2d[i])) {
			ed.MarkEdit();
		}
		++i;
	}

	if (GenericImguiDrawClass(material->descriptorSet.uboData.data(), classDescription)) {
		ed.MarkEdit();
	}
}

} // namespace ed
