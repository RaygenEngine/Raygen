#pragma once
#include "editor/windows/EdWindow.h"

#include "engine/profiler/ProfilerSetup.h"

namespace ed {
void GenericImguiDrawEntry(PodEntry* entry);

// DO NOT USE THIS CLASS AS AN EXAMPLE!
// This class uses a generic way of handling *any* asset type
class GenericAssetEditorWindow : public AssetEditorWindow {
public:
	GenericAssetEditorWindow(PodEntry* inEntry)
		: AssetEditorWindow(inEntry)
	{
	}

	void ImguiDraw() override { GenericImguiDrawEntry(entry); }
};


} // namespace ed
