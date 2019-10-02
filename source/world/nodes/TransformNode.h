#pragma once

#include "world/nodes/Node.h"

class TransformNode : public Node
{
	REFLECTED_NODE(TransformNode, Node) {}

public:
	TransformNode(Node* parent);
	~TransformNode() = default;

	std::string ToString(bool verbose, uint depth) const override;

	void ToString(std::ostream& os) const override { os << "node-type: TransformNode, name: " << GetName(); }
};
