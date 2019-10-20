#pragma once

#include "world/nodes/Node.h"
#include "asset/pods/ModelPod.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node, DF_FLAGS(ModelChange)) { REFLECT_VAR(m_model).OnDirty(DF::ModelChange); }

	PodHandle<ModelPod> m_model;

	// world space
	Box m_aabb;
	Box m_localBB;

	void CalculateAABB();

public:
	GeometryNode();

	[[nodiscard]] PodHandle<ModelPod> GetModel() const { return m_model; }
	[[nodiscard]] Box GetAABB() const { return m_aabb; }

	void SetModel(PodHandle<ModelPod> newModel);

	void DirtyUpdate(DirtyFlagset dirtyFlags) override;
};
