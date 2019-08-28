#pragma once

#include "world/nodes/Node.h"
#include "assets/model/Model.h"

namespace World
{
	class TriangleModelGeometryNode : public Node
	{
		std::shared_ptr<Assets::Model> m_model;

	public:
		TriangleModelGeometryNode(Node* parent);
		~TriangleModelGeometryNode() = default;

		Assets::Model* GetModel() const { return m_model.get(); }

		std::string ToString(bool verbose, uint depth) const override;

		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

		void ToString(std::ostream& os) const override { os << "node-type: TriangleModelGeometryNode, name: " << m_name; }
	};
}
