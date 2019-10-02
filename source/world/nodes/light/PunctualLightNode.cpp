#include "pch.h"

#include "world/nodes/light/PunctualLightNode.h"


PunctualLightNode::PunctualLightNode(Node* parent)
	: Node(parent),
	  m_color(),
      m_intensity(0.f)
{
}

std::string PunctualLightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--light " + Node::ToString(verbose, depth);
}

