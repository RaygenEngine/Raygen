#ifndef TRIANGLEMODELINSTANCEDGEOMETRYNODE_H
#define TRIANGLEMODELINSTANCEDGEOMETRYNODE_H

#include "TriangleModelGeometryNode.h"
#include "Instancing.h"

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

	public:
		void ToString(std::ostream& os) const { os << "asset-type: TriangleModelInstancedGeometryNode, name: " << m_name; }
	};

}

#endif // TRIANGLEMODELINSTANCEDGEOMETRYNODE_H
