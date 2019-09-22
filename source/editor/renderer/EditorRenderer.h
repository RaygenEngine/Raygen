#pragma once

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "world/nodes/geometry/GeometryNode.h"
class EditorRenderer : public OpenGL::GLTestRenderer
{
	MAKE_METADATA(EditorRenderer)
public:
	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);
	DECLARE_EVENT_LISTENER(m_nodeRemovedListener, Event::OnWorldNodeRemoved);

	EditorRenderer()
	{
		m_nodeAddedListener.Bind([&](Node* node) 
		{
			if (auto geometry = dynamic_cast<GeometryNode*>(node))
			{
				m_geometryObservers.emplace_back(CreateObserver<GLTestRenderer, OpenGL::GLTestGeometry>(this, geometry));
			}
		});

		m_nodeRemovedListener.Bind([&](Node* node)
		{
			if (auto geometry = dynamic_cast<GeometryNode*>(node))
			{
				auto it = std::find_if(m_geometryObservers.begin(), m_geometryObservers.end(), [node](std::shared_ptr<OpenGL::GLTestGeometry>& ptr)
				{
					return ptr.get()->GetNode() == node;
				});
				assert(it != m_geometryObservers.end() && "Attempting to remove untracked geometry node from observer list.");
				m_geometryObservers.erase(it);
			}
		});
	}

	

	virtual void Render() override;
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	virtual ~EditorRenderer() override;
};
