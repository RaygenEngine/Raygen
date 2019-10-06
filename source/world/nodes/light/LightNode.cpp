#include "pch.h"

#include "world/nodes/light/LightNode.h"


LightNode::LightNode(Node* parent)
	: Node(parent),
	  m_color(),
      m_intensity(0.f)
{}

void LightNode::DirtyUpdate()
{
	Node::DirtyUpdate();

}