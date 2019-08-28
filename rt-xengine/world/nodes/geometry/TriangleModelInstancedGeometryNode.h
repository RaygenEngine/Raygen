#pragma once

#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "world/nodes/geometry/Instancing.h"

namespace World
{
	class TriangleModelInstancedGeometryNode : public TriangleModelGeometryNode
	{
		// world matrices are stored per instance basis
		InstanceGroup m_instanceGroup;

	public:
		TriangleModelInstancedGeometryNode(Node* parent);
		~TriangleModelInstancedGeometryNode() = default;

		const InstanceGroup& GetInstanceGroup() const { return m_instanceGroup; }

		void CacheWorldTransform() override;

		std::string ToString(bool verbose, uint depth) const override;

	public:
		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
		bool LoadChildrenFromXML(const tinyxml2::XMLElement* xmlData);

		void ToString(std::ostream& os) const { os << "node-type: TriangleModelInstancedGeometryNode, name: " << m_name; }
	};

}
