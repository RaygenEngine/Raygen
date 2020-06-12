#pragma once
#include "editor/windows/EdWindow.h"

#include "editor/windows/editors/EdShaderStageEditorWindow.h"
#include "engine/profiler/ProfilerSetup.h"
#include "assets/pods/Material.h"

namespace ed {

class MaterialArchetypeEditorWindow : public AssetEditorWindowTemplate<MaterialArchetype> {
	UniquePtr<GenericShaderEditor> editor;

	UniquePtr<TextEditor> uniformEditor;
	DynamicDescriptorSetLayout editingDescSet;

public:
	MaterialArchetypeEditorWindow(PodEntry* inEntry);

	void ImguiDraw() override;

	void OnSave();
	void OnCompile();
};


class MaterialInstanceEditorWindow : public AssetEditorWindowTemplate<MaterialInstance> {

public:
	MaterialInstanceEditorWindow(PodEntry* inEntry)
		: AssetEditorWindowTemplate(inEntry)
	{
	}

	void ImguiDraw() override;
};


} // namespace ed
