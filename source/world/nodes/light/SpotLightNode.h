#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/light/LightNode.h"

class SpotLightNode : public LightNode
{
	// TODO: 

public:
	SpotLightNode(Node* parent);
	~SpotLightNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
};
