#pragma once

#include "world/nodes/Node.h"

class TransformNode : public Node
{
	// Loads from xml same as base class
public:
	TransformNode(Node* parent);
	~TransformNode() = default;

	std::string ToString(bool verbose, uint depth) const override;

	void ToString(std::ostream& os) const override { os << "node-type: TransformNode, name: " << GetName(); }
};
