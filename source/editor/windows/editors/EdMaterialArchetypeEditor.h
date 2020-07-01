#pragma once
#include "editor/windows/EdWindow.h"

#include "editor/windows/editors/EdShaderStageEditorWindow.h"
#include "engine/profiler/ProfilerSetup.h"
#include "assets/pods/MaterialInstance.h"

namespace ed {
class MaterialArchetypeEditorWindow : public AssetEditorWindowTemplate<MaterialArchetype> {

	struct ShaderEditorTab {
		// CHECK: Is pointer to member actually the best way to implement this?
		// Pointer to a MaterialArchetype str::string, used to store what field each editor is responsible for.
		using MemberStringT = decltype(&MaterialArchetype::gbufferFragMain);

		UniquePtr<TextEditor> editor;
		std::string title;
		MemberStringT textField;
		PodHandle<MaterialArchetype> handle;

		bool hasError{ false };


		ShaderEditorTab(const std::string& inTitle, PodHandle<MaterialArchetype> inHandle, MemberStringT inTextField);

		ShaderEditorTab(ShaderEditorTab&&) = default;
		ShaderEditorTab& operator=(ShaderEditorTab&&) = default;
	};

	UniquePtr<TextEditor> uniformEditor;
	DynamicDescriptorSetLayout editingDescSet;

	std::vector<ShaderEditorTab> editors;

	bool liveUpdates{ true };
	bool needsSave{ false };
	bool outputToConsole{ false };

public:
	MaterialArchetypeEditorWindow(PodEntry* inEntry);

	void FillDefaultsIfNew();

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
