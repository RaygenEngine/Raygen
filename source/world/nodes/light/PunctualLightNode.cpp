#include "pch.h"

#include "world/nodes/light/PunctualLightNode.h"


PunctualLightNode::PunctualLightNode(Node* parent)
	: LightNode(parent)
{
}

std::string PunctualLightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--PunctualLight " + Node::ToString(verbose, depth);
}

