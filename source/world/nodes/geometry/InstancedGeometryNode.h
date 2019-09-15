#pragma once

#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/geometry/Instancing.h"


class InstancedGeometryNode : public GeometryNode
{
	// world matrices are stored per instance basis
	InstanceGroup m_instanceGroup;

public:
	InstancedGeometryNode(Node* parent)
		: GeometryNode(parent) {}
	~InstancedGeometryNode() = default;

	const InstanceGroup& GetInstanceGroup() const { return m_instanceGroup; }

	void CacheWorldTransform() override;

	std::string ToString(bool verbose, uint depth) const override;

	public:
		bool LoadChildrenFromXML(const tinyxml2::XMLElement* xmlData);

	void ToString(std::ostream& os) const { os << "node-type: TriangleModelInstancedGeometryNode, name: " << GetName(); }
};
