#pragma once

#include "world/nodes/Node.h"
#include "assets/texture/CubeMap.h"

class SkyCubeNode : public Node
{
	std::shared_ptr<CubeMap> m_cubeMap;

public:
	SkyCubeNode(Node* parent);
	~SkyCubeNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	CubeMap* GetSkyMap() const { return m_cubeMap.get(); }

protected:
	std::string ToString(bool verbose, uint depth) const override;

public:

	void ToString(std::ostream& os) const override { os << "node-type: SkyCubeNode, name: " << m_name; }
};
