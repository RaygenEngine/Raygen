#pragma once

#include "renderer/renderers/opengl/test/GLTestRenderer.h"

class EditorRenderer : public OpenGL::GLTestRenderer
{
	MAKE_METADATA(EditorRenderer)
public:
	virtual void Render() override;
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	virtual ~EditorRenderer() override;
};
