#include "pch.h"

#include "world/nodes/light/DirectionalLightNode.h"


DirectionalLightNode::DirectionalLightNode(Node* parent)
	: LightNode(parent)
{
}

std::string DirectionalLightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--DirectionalLight " + Node::ToString(verbose, depth);
}

