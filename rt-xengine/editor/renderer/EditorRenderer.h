#pragma once

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"
class EditorRenderer : public OpenGL::GLTestRenderer
{
	MAKE_METADATA(EditorRenderer)
public:
	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);

	EditorRenderer()
	{
		m_nodeAddedListener.Bind([&](Node* node) 
		{
			if (auto geometry = dynamic_cast<TriangleModelGeometryNode*>(node))
			{
				m_geometryObservers.emplace_back(CreateObserver<GLTestRenderer, OpenGL::GLTestGeometry>(this, geometry));
			}
		});
	}

	virtual void Render() override;
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	virtual ~EditorRenderer() override;
};