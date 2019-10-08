#include "pch.h"
#include "editor/renderer/EditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/Editor.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"

void ForwardEditorRenderer::OnNodePodsDirty(Node* node)
{
	// if (auto geometry = dynamic_cast<GeometryNode*>(node))
	//{
	//	auto it = std::find_if(m_glGeometries.begin(), m_glGeometries.end(),
	//[node](std::unique_ptr<OpenGL::GLBasicGeometry>& ptr)
	//	{
	//		return ptr.get()->node == node;
	//	});
	//	assert(it != m_glGeometries.end() && "Attempting to update untracked geometry node from observer list.");
	//
	//	(*it)->ReloadModel();
	//}
}

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


//
// WIP:
//

void DeferredEditorRenderer::OnNodePodsDirty(Node* node)
{
	// if (auto geometry = dynamic_cast<GeometryNode*>(node))
	//{
	//	auto it = std::find_if(m_glGeometries.begin(), m_glGeometries.end(),
	//[node](std::unique_ptr<OpenGL::GLBasicGeometry>& ptr)
	//	{
	//		return ptr.get()->node == node;
	//	});
	//	assert(it != m_glGeometries.end() && "Attempting to update untracked geometry node from observer list.");

	//	(*it)->ReloadModel();
	//}
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
