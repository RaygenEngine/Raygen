#include "pch/pch.h"

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"

namespace ogl {
bool GLEditorRenderer::InitRendering(HWND assochWnd, HINSTANCE instance)
{
	if (!GLRendererBase::InitRendering(assochWnd, instance)) {
		return false;
	}
	if (Engine::IsEditorEnabled()) {
		ImguiImpl::InitOpenGL();
	}
	return true;
}

void GLEditorRenderer::Render()
{
	if (Engine::IsEditorActive()) {
		ImguiImpl::RenderOpenGL();
	}
}

GLEditorRenderer::~GLEditorRenderer()
{
	if (Engine::IsEditorEnabled()) {
		ImguiImpl::CleanupOpenGL();
	}
}

} // namespace ogl
