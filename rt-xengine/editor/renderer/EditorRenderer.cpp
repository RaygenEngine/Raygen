#include "pch.h"
#include "editor/renderer/EditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"

void EditorRenderer::Render()
{
	OpenGL::GLTestRenderer::Render();

	ImguiImpl::RenderOpenGL();
}


bool EditorRenderer::InitRendering(HWND assochWnd, HINSTANCE instance)
{
	if (!OpenGL::GLTestRenderer::InitRendering(assochWnd, instance)) 
	{
		return false;
	}
	ImguiImpl::InitOpenGL();
	return true;
}

EditorRenderer::~EditorRenderer()
{
	ImguiImpl::CleanupOpenGL();
}
