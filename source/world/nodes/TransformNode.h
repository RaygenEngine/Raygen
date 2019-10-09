#pragma once

#include "world/nodes/Node.h"

class TransformNode : public Node {
	REFLECTED_NODE(TransformNode, Node) {}

public:
	TransformNode(Node* parent)
		: Node(parent)
	{
	}
};
