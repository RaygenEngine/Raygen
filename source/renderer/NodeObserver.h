#pragma once

#include "world/nodes/Node.h"

class Renderer;

template<typename RendererT>
class RendererObject : public Object
{
/*	static_assert(!std::is_base_of_v<RendererT, Renderer>,
				  "Renderer object should refer to a valid renderer type");*/
	using Type = RendererT;
};

// saves the time of writing a base observer for each new and existing node type
template <typename NodeType, typename RendererType = Renderer>
class NodeObserver : public RendererObject<RendererType>
{
public:
	using NT = NodeType;

protected:
	NodeType* m_node;

public:
	NodeObserver(NodeType* node)
		: m_node(node)
	{
		SetName(node->GetName());
	}

	virtual void UpdateFromNode() {};

	virtual ~NodeObserver() = default;

	const NodeType* GetNode() const { return m_node; }
};

