#include "pch.h"

#include "renderer/NodeObserver.h"

namespace Renderer
{
	NodeObserver::NodeObserver(World::Node* node)
		: m_nodeBase(node)
	{
	}

	void NodeObserver::UpdateFromNode()
	{
	}
}
