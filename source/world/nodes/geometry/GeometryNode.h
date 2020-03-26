#pragma once

#include "world/nodes/Node.h"
#include "assets/pods/ModelPod.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node, DF_FLAGS(ModelChange))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_model).OnDirty(DF::ModelChange);
	}

	PodHandle<ModelPod> m_model;


public:
	GeometryNode();
	~GeometryNode() override;

	[[nodiscard]] PodHandle<ModelPod> GetModel() const { return m_model; }

	void SetModel(PodHandle<ModelPod> newModel);

	void DirtyUpdate(DirtyFlagset dirtyFlags) override;

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<typename SceneGeometry>(sceneUid, l);
	}
};
