#pragma once

#include "renderer/renderers/opengl/deferred/GLDeferredRenderer.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"
#include "world/nodes/geometry/GeometryNode.h"

// WIP:
class ForwardEditorRenderer : public OpenGL::GLForwardRenderer {
	MAKE_METADATA(ForwardEditorRenderer)
public:
	void Render() override;
	bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	~ForwardEditorRenderer() override;
};

// WIP:
class DeferredEditorRenderer : public OpenGL::GLDeferredRenderer {
	MAKE_METADATA(DeferredEditorRenderer)
public:
	void Render() override;
	bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	~DeferredEditorRenderer() override;
};
