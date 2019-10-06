#pragma once

#include "world/nodes/Node.h"
#include <functional>
class Renderer;

template<typename RendererT>
struct RendererObject
{
	static_assert(!std::is_base_of_v<RendererT, Renderer>, "Renderer object should refer to a valid renderer type");
	using Type = RendererT;
};

struct NodeObserverBase
{
	NodeObserverBase(Node* node)
		: baseNode(node) {}

	Node* baseNode;
	std::function<void(NodeObserverBase*)> onObserveeLost;

	virtual void DirtyNodeUpdate(std::bitset<64> nodeDirtyFlagset) = 0;
};


// saves the time of writing a base observer for each new and existing node type
template <typename NodeTypeT, typename RendererTypeT = Renderer>
struct NodeObserver : NodeObserverBase, RendererObject<RendererTypeT>
{
	using NodeType = NodeTypeT;
	using RendererType = RendererTypeT;
	NodeType* node;

public:
	NodeObserver(NodeType* node)
		: NodeObserverBase(node)
		, node(node) {}

	virtual void DirtyNodeUpdate(std::bitset<64> nodeDirtyFlagset) = 0;

	virtual ~NodeObserver() = default;
};

