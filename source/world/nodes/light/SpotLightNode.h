#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/light/LightNode.h"

class SpotLightNode : public LightNode
{
	// TODO: 

public:
	SpotLightNode(Node* parent);
	~SpotLightNode() = default;

	std::string ToString(bool verbose, uint depth) const override;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	void ToString(std::ostream& os) const override { os << "node-type: SpotLightNode, name: " << m_name; }
};
