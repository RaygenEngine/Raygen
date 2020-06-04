#include "pch.h"
#include "EdMaterialArchetypeEditor.h"
#include "assets/PodEditor.h"
#include "editor/imgui/ImEd.h"
#include "assets/pods/Material.h"
#include "assets/AssetRegistry.h"
#include "assets/util/SpirvReflector.h"
#include "assets/util/SpirvCompiler.h"

#include "editor/windows/general/EdPropertyEditorWindow.h"

namespace ed {

void MaterialArchetypeEditorWindow::ImguiDraw()
{
	OptionalPodEditor ed(podHandle);
	auto archetype = ed.BeginOptionalEditRegion();


	auto shaderEntry = AssetHandlerManager::GetEntry(archetype->fragShader);
	ImEd::Button(shaderEntry->name.c_str());
	if (auto entry = ImEd::AcceptTypedPodDrop<ShaderStage>(); entry) {
		auto handle = entry->GetHandleAs<ShaderStage>();
		if (!handle.IsDefault() && handle.Lock()->stage == ShaderStageType::Fragment) {
			archetype->fragShader = entry->GetHandleAs<ShaderStage>();
			ed.MarkEdit();
		}
	}

	if (ImEd::Button("Redo Reflection")) {
		auto shader = archetype->fragShader.Lock();
		if (shader->binary.empty()) {
			PodEditor fragEditor(archetype->fragShader);
			auto mutShader = fragEditor.pod;
			mutShader->binary = ShaderCompiler::Compile(shader->code, ShaderStageType::Fragment);
		}


		archetype->parameters = SpirvReflector::ReflectArchetype(shader->binary);
		archetype->RegenerateClass();
	}

	auto& uboMembers = archetype->parameters.uboMembers;
	auto& samplers = archetype->parameters.samplers2d;

	if (ImGui::CollapsingHeader("Textures")) {
		for (auto& sampler : samplers) {
			ImGui::TextUnformatted(sampler.c_str());
		}
	}

	for (auto& mem : uboMembers) {
		ImGui::Text(
			"%d\t%s\t%s", mem.SizeOf(), std::string(GenMetaEnum(mem.type).GetValueStr()).c_str(), mem.name.c_str());
	}
}


void MaterialInstanceEditorWindow::ImguiDraw()
{
	OptionalPodEditor ed(podHandle);
	auto material = ed.BeginOptionalEditRegion();

	auto archetype = material->archetype.Lock();
	auto archetypeEntry = AssetHandlerManager::GetEntry(material->archetype);
	ImEd::Button(archetypeEntry->name.c_str());


	bool shouldReload = false;

	if (auto entry = ImEd::AcceptTypedPodDrop<MaterialArchetype>(); entry) {
		material->archetype = entry->GetHandleAs<MaterialArchetype>();
		ed.MarkEdit();
		shouldReload = true;
		material->RegenerateUbo();
	}

	if (archetype->parameters.samplers2d.size() != material->samplers2d.size()) {
		material->samplers2d.resize(archetype->parameters.samplers2d.size());
		ed.MarkEdit();
		material->RegenerateUbo();
	}

	RuntimeClass* classDescription = material->archetype.Lock()->classDescr.get();
	if (!classDescription) {
		ImGui::Text("Generate the class of the archetype first.");
		if (ImEd::Button("GENERATE")) {
			PodEditor archetypeEditor(material->archetype);
			archetypeEditor.pod->RegenerateClass();
		}
		return;
	}

	if (material->uboData.size() != classDescription->GetSize()) {
		material->RegenerateUbo();
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
