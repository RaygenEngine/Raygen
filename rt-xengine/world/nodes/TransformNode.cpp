#include "pch.h"

#include "world/nodes/TransformNode.h"

TransformNode::TransformNode(Node* parent)
	: Node(parent)
{
}

std::string TransformNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--transform " + Node::ToString(verbose, depth);
}
