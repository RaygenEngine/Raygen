#include "pch.h"
#include "editor/renderer/EditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/Editor.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"

void EditorRenderer::OnNodePodsDirty(Node* node)
{
	if (auto geometry = dynamic_cast<GeometryNode*>(node))
	{
		auto it = std::find_if(m_glGeometries.begin(), m_glGeometries.end(), [node](std::unique_ptr<OpenGL::GLBasicGeometry>& ptr)
		{
			return ptr.get()->GetNode() == node;
		});
		assert(it != m_glGeometries.end() && "Attempting to update untracked geometry node from observer list.");
		
		(*it)->ReloadModel();
	}
}

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
