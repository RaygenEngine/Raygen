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
struct NodeObserver : RendererObject<RendererType>
{
	using NT = NodeType;

	NodeType* node;

public:
	NodeObserver(NodeType* node)
		: node(node)
	{
		SetName(node->GetName());
	}

	virtual void UpdateFromNode() {};

	virtual ~NodeObserver() = default;
};

