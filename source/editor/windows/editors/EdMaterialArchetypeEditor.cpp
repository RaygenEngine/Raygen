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
			uboText << "sampler2d " << sampler << ";\n";
		}
		return uboText;
	}

	// Returns true if the "compilation" was successful
	bool ValidateUniforms(TextEditor& uniformEditor, DynamicDescriptorSetLayout& layout)
	{
		TextEditor::ErrorMarkers errors;

		std::unordered_set<std::string, str::Hash> identifiers;
		layout = {};

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


MaterialArchetypeEditorWindow::ShaderEditorTab::ShaderEditorTab(
	const std::string& inTitle, PodHandle<MaterialArchetype> inHandle, MemberStringT inTextField)
	: title(inTitle)
	, textField(inTextField)
	, handle(inHandle)
{
	editor.reset(new TextEditor());

	if ((handle.Lock()->*inTextField).size() > 0) {
		editor->SetText((handle.Lock()->*inTextField));
	}
	editor->SetColorizerEnable(true);
	editor->SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	editor->SetShowWhitespaces(false);
}

MaterialArchetypeEditorWindow::MaterialArchetypeEditorWindow(PodEntry* inEntry)
	: AssetEditorWindowTemplate(inEntry)
{
	auto addEditor = [&](const std::string& tabName, ShaderEditorTab::MemberStringT field) {
		editors.emplace_back(ShaderEditorTab(tabName, podHandle, field));
	};

	addEditor("Shared", &MaterialArchetype::sharedFunctions);
	addEditor("Fragment", &MaterialArchetype::gbufferFragMain);
	addEditor("Depth", &MaterialArchetype::depthShader);


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

	ImGui::Checkbox("Live Updates", &liveUpdates);
	ImGui::SameLine();
	ImGui::Checkbox("Output To Console", &outputToConsole);
	if (needsSave) {
		ImGui::SameLine();
		if (ImEd::Button(ETXT(FA_SAVE, "Save"))) {
			OnSave();
		}
	}

	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(83)) {
		OnSave();
	}

	if (ImGui::BeginTabBar("ShaderEditorTabs")) {
		for (int i = 0; auto& tab : editors) {
			if (tab.hasError) {
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(0xA02040ff));
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(0xA02040ff));
			}

			if (ImGui::BeginTabItem(tab.title.c_str())) {
				ImGui::PushID(i++);
				ImEd::BeginCodeFont();
				tab.editor->Render(tab.title.c_str());
				ImEd::EndCodeFont();
				ImGui::PopID();
				ImGui::EndTabItem();
				if (tab.editor->IsTextChanged()) {
					needsSave = true;
				}
			}

			if (tab.hasError) {
				ImGui::PopStyleColor(2);
			}
		}

		ImGui::EndTabBar();
	}
}

void MaterialArchetypeEditorWindow::OnSave()
{
	needsSave = false;
	OnCompile();
	SaveToDisk();
}

void MaterialArchetypeEditorWindow::OnCompile()
{
	PodEditor ed(podHandle);

	for (auto& tab : editors) {
		*(ed.pod).*tab.textField = tab.editor->GetText();
	}

	if (!ValidateUniforms(*uniformEditor, editingDescSet)) {
		return;
	}
	shd::GeneratedShaderErrors errors;


	if (ed->CompileAll(std::move(editingDescSet), errors, outputToConsole)) {
		for (auto& tab : editors) {
			tab.hasError = false;
			tab.editor->SetErrorMarkers({});
		}
		return;
	}

	for (auto& tab : editors) {
		if (auto r = errors.editorErrors.find(tab.title); r != errors.editorErrors.end()) {
			if (r->second.errors.size() > 0) {
				tab.editor->SetErrorMarkers(r->second.errors);
				tab.hasError = true;
			}
		}
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

	if (archetype->descriptorSetLayout.samplers2d.size() != material->descriptorSet.samplers2d.size()) {
		ImGui::Text("Incorrect samplers count!");
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
