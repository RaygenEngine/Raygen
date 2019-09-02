#include "pch.h"

#include "world/nodes/light/LightNode.h"
#include "assets/other/xml/ParsingAux.h"

namespace World
{
	LightNode::LightNode(Node* parent)
		: Node(parent),
		  m_color()
	{
		REFLECT_VAR(m_color);
	}

	std::string LightNode::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--light " + Node::ToString(verbose, depth);
	}
}
