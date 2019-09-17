#include "pch.h"
#include "editor/renderer/EditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/Editor.h"

void EditorRenderer::Render()
{
	OpenGL::GLTestRenderer::Render();
	if (Engine::GetEditor()->m_showImgui)
	{
		ImguiImpl::RenderOpenGL();
	}
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
