#include "pch.h"
#include "TransformNode.h"


namespace World
{
	TransformNode::TransformNode(Node* parent)
		: Node(parent)
	{
	}

	std::string TransformNode::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--transform " + Node::ToString(verbose, depth);
	}
}
