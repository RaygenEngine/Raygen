#include "pch/pch.h"
#include "editor/renderer/EditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/Editor.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"

void ForwardEditorRenderer::Render()
{
	OpenGL::GLForwardRenderer::Render();
	if (Engine::GetEditor()->m_showImgui) {
		ImguiImpl::RenderOpenGL();
	}
}


bool ForwardEditorRenderer::InitRendering(HWND assochWnd, HINSTANCE instance)
{
	if (!OpenGL::GLForwardRenderer::InitRendering(assochWnd, instance)) {
		return false;
	}
	ImguiImpl::InitOpenGL();
	return true;
}

ForwardEditorRenderer::~ForwardEditorRenderer()
{
	ImguiImpl::CleanupOpenGL();
}

void DeferredEditorRenderer::Render()
{
	OpenGL::GLDeferredRenderer::Render();
	if (Engine::GetEditor()->m_showImgui) {
		ImguiImpl::RenderOpenGL();
	}
}

bool DeferredEditorRenderer::InitRendering(HWND assochWnd, HINSTANCE instance)
{
	if (!OpenGL::GLDeferredRenderer::InitRendering(assochWnd, instance)) {
		return false;
	}
	ImguiImpl::InitOpenGL();
	return true;
}

DeferredEditorRenderer::~DeferredEditorRenderer()
{
	ImguiImpl::CleanupOpenGL();
}
