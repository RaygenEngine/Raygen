#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/light/LightNode.h"

class PunctualLightNode : public LightNode
{
	REFLECTED_NODE(PunctualLightNode, LightNode) {}

public:
	PunctualLightNode(Node* parent);
	~PunctualLightNode() = default;
};
