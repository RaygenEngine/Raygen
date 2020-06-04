#include "pch.h"
#include "EdMaterialArchetypeEditor.h"
#include "assets/PodEditor.h"
#include "editor/imgui/ImEd.h"
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

	if (ImGui::CollapsingHeader("Textures")) {
		for (auto& sampler : samplers) {
			ImGui::TextUnformatted(sampler.c_str());
		}
	}

	for (auto& mem : uboMembers) {
		ImGui::Text(
			"%d\t%s\t%s", mem.SizeOf(), std::string(GenMetaEnum(mem.type).GetValueStr()).c_str(), mem.name.c_str());
	}

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
	ImEd::Button(archetypeEntry->name.c_str());

	if (auto entry = ImEd::AcceptTypedPodDrop<MaterialArchetype>(); entry) {
		{
			PodEditor arch(material->archetype);
			auto it = std::remove(arch->instances.begin(), arch->instances.end(), podHandle);
			if (it != arch->instances.end()) {
				arch->instances.erase(it);
			}
		}
		material->archetype = entry->GetHandleAs<MaterialArchetype>();

		PodEditor arch(material->archetype);
		arch->instances.push_back(podHandle);
		arch->OnShaderUpdated();
		ed.MarkEdit();
	}


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
		ImGui::Button(img.c_str());
		ImGui::SameLine();
		ImGui::Button(AssetHandlerManager::GetEntry(material->samplers2d[i])->path.c_str());

		if (auto entry = ImEd::AcceptTypedPodDrop<Image>(); entry) {
			auto handle = entry->GetHandleAs<Image>();
			ed.MarkEdit();
			material->samplers2d[i] = handle;
		}

		++i;
	}

	if (GenericImguiDrawClass(material->uboData.data(), *classDescription)) {
		ed.MarkEdit();
	}
}

} // namespace ed
