#pragma once
#include "assets/pods/ModelPod.h"
#include "universe/nodes/Node.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node, DF_FLAGS(ModelChange))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_model).OnDirty(DF::ModelChange);
	}

	PodHandle<Model> m_model;


public:
	GeometryNode();
	~GeometryNode() override;

	[[nodiscard]] PodHandle<Model> GetModel() const { return m_model; }

	void SetModel(PodHandle<Model> newModel);

	void DirtyUpdate(DirtyFlagset dirtyFlags) override;

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<typename SceneGeometry>(sceneUid, l);
	}
};
