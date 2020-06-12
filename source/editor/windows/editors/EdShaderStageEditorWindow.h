#pragma once
#include "editor/windows/EdWindow.h"

#include "assets/pods/ShaderStage.h"

class TextEditor;
namespace ed {

struct GenericShaderEditor {

	UniquePtr<TextEditor> editor;
	bool needsSave{ false };
	bool liveUpdates{ true };

	GenericShaderEditor(const std::string& initialCode, const std::string& filepath, std::function<void()>&& onSaveCb,
		std::function<void()> onUpdateCb);

	std::function<void()> onSave;
	std::function<void()> onUpdate;

	std::string filepathTitle;

	void ImguiDraw();
};


class ShaderStageEditorWindow : public AssetEditorWindowTemplate<ShaderStage> {

	UniquePtr<GenericShaderEditor> editor;

	std::string filepathCache;
	void SaveInternal();

	void UpdateForGpu();

public:
	ShaderStageEditorWindow(PodEntry* inEntry);

	void ImguiDraw() override;
};


} // namespace ed
