#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"

namespace ogl {
class GLEditorRenderer : public GLRendererBase {

public:
	~GLEditorRenderer() override;

	void InitRendering(HWND assochWnd, HINSTANCE instance) override;

	// When calling this from children renderers, call it after you are done with your drawing, otherwise you may draw
	// on top of the editor
	void Render() override;

	bool SupportsEditor() override { return true; }
};
} // namespace ogl
