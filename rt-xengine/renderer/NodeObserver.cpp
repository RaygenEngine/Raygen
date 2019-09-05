#include "pch.h"

#include "renderer/NodeObserver.h"

namespace Renderer
{
	NodeObserver::NodeObserver(Node* node)
		: m_nodeBase(node)
	{
	}

	void NodeObserver::UpdateFromNode()
	{
	}
}
