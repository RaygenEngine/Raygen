#pragma once
#include "editor/windows/EdWindow.h"

struct PodEntry;
namespace ed {
class ShaderEditorWindow : public UniqueWindow {
public:
	ShaderEditorWindow(std::string_view name);

	void OpenShaderForEditing(PodEntry* entry, bool isFrag);

	virtual void ImguiDraw();
	virtual ~ShaderEditorWindow() = default;

private:
	PodEntry* openShader{ nullptr };
};

} // namespace ed
