#include "pch/pch.h"

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"

namespace ogl {
void GLEditorRenderer::InitRendering(HWND assochWnd, HINSTANCE instance)
{
	GLRendererBase::InitRendering(assochWnd, instance);

	if (Engine::IsEditorEnabled()) {
		ImguiImpl::InitOpenGL();
	}
}

void GLEditorRenderer::Render()
{
	GLRendererBase::Render();

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
