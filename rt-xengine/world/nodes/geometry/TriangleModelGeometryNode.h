#pragma once

#include "world/nodes/Node.h"
#include "assets/model/Model.h"


class TriangleModelGeometryNode : public Node
{
	Model* m_model;

public:
	TriangleModelGeometryNode(Node* parent);
	~TriangleModelGeometryNode() = default;

	Model* GetModel() const { return m_model; }

	std::string ToString(bool verbose, uint depth) const override;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	void ToString(std::ostream& os) const override { os << "node-type: TriangleModelGeometryNode, name: " << GetName(); }
};
