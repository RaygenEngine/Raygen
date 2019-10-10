#pragma once

#include "renderer/renderers/opengl/deferred/GLDeferredRenderer.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"
#include "world/nodes/geometry/GeometryNode.h"

// WIP:
class ForwardEditorRenderer : public OpenGL::GLForwardRenderer {
public:
	void Render() override;
	bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	~ForwardEditorRenderer() override;
};

// WIP: TODO:
class DeferredEditorRenderer : public OpenGL::GLDeferredRenderer {
public:
	void Render() override;
	bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	~DeferredEditorRenderer() override;
};
