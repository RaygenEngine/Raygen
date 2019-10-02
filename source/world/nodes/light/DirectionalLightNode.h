#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/light/LightNode.h"


class DirectionalLightNode : public LightNode
{

public:
	DirectionalLightNode(Node* parent);
	~DirectionalLightNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
	
	std::string ToString(bool verbose, uint depth) const override;

	void ToString(std::ostream& os) const override { os << "node-type: DirectionalLightNode, name: " << m_name; }
};
