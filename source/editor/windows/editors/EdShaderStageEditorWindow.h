#pragma once
#include "editor/windows/EdWindow.h"

#include "assets/pods/ShaderStage.h"

class TextEditor;
namespace ed {


class ShaderStageEditorWindow : public AssetEditorWindowTemplate<ShaderStage> {
	UniquePtr<TextEditor> editor;

	std::string filepathCache;
	bool needsSave{ false };

	void SaveInternal();

public:
	ShaderStageEditorWindow(PodEntry* inEntry);

	void ImguiDraw() override;
};


} // namespace ed
