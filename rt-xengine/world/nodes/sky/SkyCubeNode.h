#pragma once

#include "world/nodes/Node.h"
#include "assets/texture/CubeMap.h"

class SkyCubeNode : public Node
{
	CubeMap m_cubeMap;

public:
	SkyCubeNode(Node* parent);
	~SkyCubeNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	CubeMap& GetSkyMap() { return m_cubeMap; }

protected:
	std::string ToString(bool verbose, uint depth) const override;

public:

	void ToString(std::ostream& os) const override { os << "node-type: SkyCubeNode, name: " << GetName(); }
};
