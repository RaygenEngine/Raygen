#pragma once

#include "world/nodes/Node.h"
#include "asset/assets/GltfModelAsset.h"

class GeometryNode : public Node
{
	PodHandle<ModelPod> m_model;

public:
	GeometryNode(Node* parent)
		: Node(parent)
	{
		REFLECT_VAR(m_model);
	}
	~GeometryNode() = default;

	PodHandle<ModelPod> GetModel() const { return m_model; }

	std::string ToString(bool verbose, uint depth) const override;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	void ToString(std::ostream& os) const override { os << "node-type: TriangleModelGeometryNode, name: " << GetName(); }
};
