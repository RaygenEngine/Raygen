#pragma once
#include "editor/windows/EdWindow.h"

// TODO: Remove class
struct PodEntry;
namespace ed {
class ShaderEditorWindow : public UniqueWindow {
public:
	ShaderEditorWindow::ShaderEditorWindow(std::string_view name)
		: UniqueWindow(name)
	{
	}

	// void OpenShaderForEditing(PodEntry* entry, bool isFrag);

	virtual void OnDraw(const char* title, bool* keepOpen){};
	virtual ~ShaderEditorWindow() = default;

private:
	PodEntry* openShader{ nullptr };
};

} // namespace ed
