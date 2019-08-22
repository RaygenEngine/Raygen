#include "pch.h"
#include "NodeObserver.h"


namespace Renderer
{
	NodeObserver::NodeObserver(World::Node* node)
		: m_nodeBase(node)
	{
	}

	void NodeObserver::UpdateFromNode()
	{
	}

	void NodeObserver::UpdateFromVisual(RenderTarget* target)
	{
	}
}
