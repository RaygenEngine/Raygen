#include "pch.h"

#include "world/nodes/light/LightNode.h"


LightNode::LightNode(Node* parent)
	: Node(parent),
	  m_color(),
      m_intensity(0.f)
{}

std::string LightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--light " + Node::ToString(verbose, depth);
}

