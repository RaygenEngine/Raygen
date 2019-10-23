#pragma once

#include "world/nodes/Node.h"
#include "asset/pods/ModelPod.h"
#include "core/MathAux.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node, DF_FLAGS(ModelChange)) { REFLECT_VAR(m_model).OnDirty(DF::ModelChange); }

	PodHandle<ModelPod> m_model;

	// world space
	math::AABB m_aabb;
	math::AABB m_localBB;

	void CalculateAABB();

public:
	GeometryNode();

	[[nodiscard]] PodHandle<ModelPod> GetModel() const { return m_model; }
	[[nodiscard]] math::AABB GetAABB() const { return m_aabb; }
	[[nodiscard]] math::AABB GetLocalAABB() const { return m_localBB; }

	void SetModel(PodHandle<ModelPod> newModel);

	void DirtyUpdate(DirtyFlagset dirtyFlags) override;
};
