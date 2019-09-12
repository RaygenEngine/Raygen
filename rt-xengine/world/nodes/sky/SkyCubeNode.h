#pragma once

#include "world/nodes/Node.h"
#include "assets/texture/CubeMapAsset.h"

class SkyCubeNode : public Node
{
	CubeMapAsset* m_cubeMap;

public:
	SkyCubeNode(Node* parent)
		: Node(parent),
	      m_cubeMap(nullptr) {}

	~SkyCubeNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	CubeMapAsset* GetSkyMap() const { return m_cubeMap; }

protected:
	std::string ToString(bool verbose, uint depth) const override;

public:

	void ToString(std::ostream& os) const override { os << "node-type: SkyCubeNode, name: " << GetName(); }
};
