#pragma once
#include "assets/pods/Mesh.h"
#include "universe/nodes/Node.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node, DF_FLAGS(ModelChange))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_model).OnDirty(DF::ModelChange);
	}

	PodHandle<Mesh> m_model;


public:
	GeometryNode();
	~GeometryNode() override;

	[[nodiscard]] PodHandle<Mesh> GetModel() const { return m_model; }

	void SetModel(PodHandle<Mesh> newModel);

	void DirtyUpdate(DirtyFlagset dirtyFlags) override;

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<typename SceneGeometry>(sceneUid, l);
	}
};
