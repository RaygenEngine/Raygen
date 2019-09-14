#pragma once

#include "world/nodes/Node.h"
#include "asset/assets/CubemapAsset.h"

class SkyCubeNode : public Node
{
	CubemapPod* m_cubeMap;

public:
	SkyCubeNode(Node* parent)
		: Node(parent),
	      m_cubeMap(nullptr) {}

	~SkyCubeNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	CubemapPod* GetSkyMap() const { return m_cubeMap; }

protected:
	std::string ToString(bool verbose, uint depth) const override;

public:

	void ToString(std::ostream& os) const override { os << "node-type: SkyCubeNode, name: " << GetName(); }
};
