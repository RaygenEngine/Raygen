#pragma once
#include "editor/windows/EdWindow.h"

#include "engine/profiler/ProfilerSetup.h"
#include "assets/pods/Material.h"

namespace ed {

class MaterialArchetypeEditorWindow : public AssetEditorWindowTemplate<MaterialArchetype> {

public:
	MaterialArchetypeEditorWindow(PodEntry* inEntry)
		: AssetEditorWindowTemplate(inEntry)
	{
	}


	void ImguiDraw() override;
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
