#include "pch.h"
#include "editor/renderer/EditorRenderer.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/Editor.h"

void EditorRenderer::OnNodePodsDirty(Node* node)
{
	if (auto geometry = dynamic_cast<GeometryNode*>(node))
	{
		auto it = std::find_if(m_geometryObservers.begin(), m_geometryObservers.end(), [node](std::shared_ptr<OpenGL::GLTestGeometry>& ptr)
		{
			return ptr.get()->GetNode() == node;
		});
		assert(it != m_geometryObservers.end() && "Attempting to update untracked geometry node from observer list.");
		std::shared_ptr<OpenGL::GLTestGeometry> ptr = *it;

		ptr->ReloadModel();
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
