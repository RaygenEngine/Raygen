#pragma once
#include "assets/pods/Mesh.h"
#include "universe/nodes/Node.h"

class GeometryNode : public Node {
	REFLECTED_NODE(GeometryNode, Node, DF_FLAGS(ModelChange))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_mesh).OnDirty(DF::ModelChange);
	}

	PodHandle<Mesh> m_mesh;

public:
	GeometryNode();
	~GeometryNode() override;

	[[nodiscard]] PodHandle<Mesh> GetModel() const { return m_mesh; }

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
