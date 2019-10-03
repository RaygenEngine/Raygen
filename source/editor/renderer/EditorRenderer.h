#pragma once

#include "renderer/renderers/opengl/deferred/GLDeferredRenderer.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"
#include "world/nodes/geometry/GeometryNode.h"

// WIP:
class ForwardEditorRenderer : public OpenGL::GLForwardRenderer
{
	MAKE_METADATA(ForwardEditorRenderer)
public:
	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);
	DECLARE_EVENT_LISTENER(m_nodeRemovedListener, Event::OnWorldNodeRemoved);

	ForwardEditorRenderer()
	{
		m_nodeAddedListener.Bind([&](Node* node) 
		{
			if (auto geometry = dynamic_cast<GeometryNode*>(node))
			{
				m_glGeometries.emplace_back(CreateObserver<OpenGL::GLBasicGeometry>(geometry));
			}
		});

		m_nodeRemovedListener.Bind([&](Node* node)
		{
			if (auto geometry = dynamic_cast<GeometryNode*>(node))
			{
				auto it = std::find_if(m_glGeometries.begin(), m_glGeometries.end(), [node](std::unique_ptr<OpenGL::GLBasicGeometry>& ptr)
				{
					return ptr.get()->node == node;
				});
				assert(it != m_glGeometries.end() && "Attempting to remove untracked geometry node from observer list.");
				m_glGeometries.erase(it);
			}
		});
	}


	void OnNodePodsDirty(Node* node);

	

	virtual void Render() override;
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	virtual ~ForwardEditorRenderer() override;
};

// WIP:
class DeferredEditorRenderer : public OpenGL::GLDeferredRenderer
{
	MAKE_METADATA(DeferredEditorRenderer)
public:
	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);
	DECLARE_EVENT_LISTENER(m_nodeRemovedListener, Event::OnWorldNodeRemoved);

	DeferredEditorRenderer()
	{
		m_nodeAddedListener.Bind([&](Node* node)
		{
			if (auto geometry = dynamic_cast<GeometryNode*>(node))
			{
				m_glGeometries.emplace_back(CreateObserver<OpenGL::GLBasicGeometry>(geometry));
			}
		});

		m_nodeRemovedListener.Bind([&](Node* node)
		{
			if (auto geometry = dynamic_cast<GeometryNode*>(node))
			{
				auto it = std::find_if(m_glGeometries.begin(), m_glGeometries.end(), [node](std::unique_ptr<OpenGL::GLBasicGeometry>& ptr)
				{
					return ptr.get()->node == node;
				});
				assert(it != m_glGeometries.end() && "Attempting to remove untracked geometry node from observer list.");
				m_glGeometries.erase(it);
			}
		});
	}


	void OnNodePodsDirty(Node* node);



	virtual void Render() override;
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	virtual ~DeferredEditorRenderer() override;
};
