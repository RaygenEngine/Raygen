#pragma once

#include "world/nodes/Node.h"
#include "renderer/Renderer.h"
class Renderer;

template<typename RendererT>
struct RendererObject {
	static_assert(!std::is_base_of_v<RendererT, Renderer>, "Renderer object should refer to a valid renderer type");
	using Type = RendererT;
};

struct NodeObserverBase : Object {
	NodeObserverBase(Node* node)
		: baseNode(node)
	{
	}

	Node* baseNode;

	virtual void DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset) = 0;
};


// saves the time of writing a base observer for each new and existing node type
template<typename NodeTypeT, typename RendererTypeT = Renderer>
struct NodeObserver
	: NodeObserverBase
	, RendererObject<RendererTypeT> {

	using NodeType = NodeTypeT;
	using RendererType = RendererTypeT;
	NodeType* node;

	NodeObserver(NodeType* node)
		: NodeObserverBase(node)
		, node(node)
	{
	}
};
