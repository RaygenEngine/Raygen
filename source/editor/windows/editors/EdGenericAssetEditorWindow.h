#pragma once
#include "editor/windows/EdWindow.h"

#include "engine/profiler/ProfilerSetup.h"

namespace ed {
void GenericImguiDrawEntry(PodEntry* entry);

// DO NOT USE THIS CLASS AS AN EXAMPLE!
// This class is templated because it supports all possible pod types
template<CONC(CAssetPod) PodType>
class GenericAssetEditorWindow : public AssetEditorWindowTemplate<PodType> {
public:
	GenericAssetEditorWindow(PodEntry* inEntry)
		: AssetEditorWindowTemplate<PodType>(inEntry)
	{
	}

	void ImguiDraw() override { GenericImguiDrawEntry(entry); }
};


} // namespace ed
