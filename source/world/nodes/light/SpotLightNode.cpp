#include "pch.h"

#include "world/nodes/light/SpotLightNode.h"


SpotLightNode::SpotLightNode(Node* parent)
	: LightNode(parent)
{
}

std::string SpotLightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SpotLight " + Node::ToString(verbose, depth);
}

