#include "EdMaterialArchetypeEditor.h"

#include "assets/PodEditor.h"
#include "assets/pods/MaterialInstance.h"
#include "editor/imgui/ImAssetSlot.h"
#include "editor/misc/NativeFileBrowser.h"
#include "editor/windows/general/EdPropertyEditorWindow.h"

#include <imguicolortextedit/TextEditor.h>

namespace ed {

namespace {

	// Returns true if the "compilation" was successful
	// TODO: Use shadercompiler::compileubo
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
	FillDefaultsIfNew();

	auto addEditor = [&](const std::string& tabName, ShaderEditorTab::MemberStringT field) {
		editors.emplace_back(ShaderEditorTab(tabName, podHandle, field));
	};

	addEditor("Shared", &MaterialArchetype::sharedFunctions);
	addEditor("Fragment", &MaterialArchetype::gbufferFragMain);
	addEditor("Depth", &MaterialArchetype::depthShader);
	addEditor("Vertex", &MaterialArchetype::gbufferVertMain);
	addEditor("Unlit", &MaterialArchetype::unlitFragMain);


	uniformEditor.reset(new TextEditor());
	uniformEditor->SetText(podHandle.Lock()->descriptorSetLayout.GetUniformText().str());
	uniformEditor->SetColorizerEnable(true);
	uniformEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	uniformEditor->SetShowWhitespaces(false);
}

void MaterialArchetypeEditorWindow::FillDefaultsIfNew()
{

	if (podHandle.Lock()->depthShader.size() == 0) {
		PodEditor ed(podHandle);
		ed->depthShader =
			R"(
void main() {}
)";
		needsSave = true;
	}

	if (podHandle.Lock()->gbufferFragMain.size() == 0) {
		PodEditor ed(podHandle);
		ed->gbufferFragMain =
			R"(
void main() {

	vec3 baseColor = vec3(0.5);
	float metalness = 0;
	float roughness = 0.5;
	vec3 emissive = vec3(0);
	float occlusion = 1.;
	float opacity = 1.; // Translucent not handled here, but we still need to write something to the gbuffer

	// This is a good sample, but you can write your own values.
	{
		// albedo = (1.0 - metalness) * baseColor;
		gAlbedo = vec4((1.0 - metalness) * baseColor, opacity);

		// CHECK: reflectance
		// f0 = 0.16 * reflectance * reflectance * (1.0 - metalness) + albedo * metalness;
		gSpecularColor = vec4(vec3(0.16 * 0.5 * 0.5 * (1.0 - metalness)) + baseColor * metalness, roughness * roughness);
	
		gEmissive = vec4(emissive, occlusion);
	}
	
	vec3 customNormal = vec3(0.5, 0.5, 1.0); // "Default" normal, can be overriden by materials if needed
	vec3 normal = normalize(customNormal * 2.0 - 1.0);

    gSNormal = vec4(normalize(TBN * normal), 1.0);

	// No reason to edit anything below here, unless you really know what you're doing.
	// geometric normal
	gGNormal = vec4(TBN[2], 1.0);
	
	vec2 a = (clipPos.xy / clipPos.w) * 0.5 + 0.5;
    vec2 b = (prevClipPos.xy / prevClipPos.w) * 0.5 + 0.5;

	float expectedDepth = (prevClipPos.z / prevClipPos.w);

	gVelocity = vec4(b - a, expectedDepth, 1.f);
	gUVDrawIndex = vec4(uv, drawIndex, 1.f);
}                                                                                        
)";
		needsSave = true;
	}
}

void MaterialArchetypeEditorWindow::ImguiDraw()
{

	{
		OptionalPodEditor podEditor(podHandle);

		auto pod = podEditor.BeginOptionalEditRegion();
		if (ImEd::EnumDropDown("Pass Type", pod->passType)) {
			podEditor.MarkEdit();
			OnPassTypeChanged();
		}
	}

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
		if (ImGui::Button(ETXT(FA_SAVE, "Save"))) {
			OnSave();
		}
	}

	if (entry->metadata.exportOnSave) {
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, EdColor::LightSuccess);
		if (ImGui::Button(ETXT(FA_FILE_EXPORT, "Backup Location"))) {
			auto opt = NativeFileBrowser::SaveFile({}, fs::relative(fs::path(entry->metadata.originalImportLocation)));
			if (opt) {
				entry->metadata.originalImportLocation = opt.value().generic_string();
			}
		}
		ImGui::PopStyleColor();
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Disable Exporting", nullptr, nullptr)) {
				entry->metadata.exportOnSave = false;
			}
			ImGui::EndPopup();
		}
		TEXT_TOOLTIP("Saving at: {}", entry->metadata.originalImportLocation);
	}
	else {
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, EdColor::LightText);
		if (ImGui::Button(ETXT(FA_FILE_EXPORT, "Set Backup Location"))) {
			if (auto opt = NativeFileBrowser::SaveFile({}); opt) {
				entry->metadata.originalImportLocation = opt.value().generic_string();
				entry->metadata.exportOnSave = true;
			}
		}
		ImGui::PopStyleColor();
		TEXT_TOOLTIP("Disabled, click to select a disk location.");
	}


	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(83)) {
		OnSave();
	}

	if (ImGui::BeginTabBar("ShaderEditorTabs")) {
		for (int i = 0; auto& tab : editors) {
			if (tab.hasError) {
				ImGui::PushStyleColor(ImGuiCol_Border, EdColor::Failure);
				ImGui::PushStyleColor(ImGuiCol_Text, EdColor::Failure);
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

void MaterialArchetypeEditorWindow::OnPassTypeChanged() {}

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
	auto archetypeEntry = AssetRegistry::GetEntry(material->archetype);
	auto prevArchetype = archetype;


	// TODO: Hidden bug? Does not get removed from previous archetype
	if (ImEd::AssetSlot("Archetype", material->archetype)) {
		ed.MarkEdit();
		archetype = material->archetype.Lock();

		PodEditor arch(material->archetype);
		if (auto it = std::find(arch->instances.begin(), arch->instances.end(), podHandle);
			it == arch->instances.end()) {
			arch->instances.push_back(podHandle);
		}
		material->descriptorSet.SwapLayout(prevArchetype->descriptorSetLayout, arch->descriptorSetLayout);
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Separator();

	const RuntimeClass& classDescription = material->archetype.Lock()->descriptorSetLayout.uboClass;

	if (material->descriptorSet.uboData.size() != classDescription.GetSize()
		|| archetype->descriptorSetLayout.samplers2d.size() != material->descriptorSet.samplers2d.size()) {

		ed.MarkEdit();

		material->descriptorSet.SwapLayout({}, material->archetype.Lock()->descriptorSetLayout);
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
