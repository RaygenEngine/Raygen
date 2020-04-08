#include "pch.h"
#include "GeometryNode.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"

GeometryNode::GeometryNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneGeometry>();
}

void GeometryNode::SetModel(PodHandle<Mesh> newModel)
{
	m_model = newModel;
	SetDirty(DF::ModelChange);
}

void GeometryNode::DirtyUpdate(DirtyFlagset dirtyFlags)
{
	Node::DirtyUpdate(dirtyFlags);

	if (dirtyFlags[DF::ModelChange]) {
		CalculateWorldAABB();

		Enqueue([model = GetModel()](SceneGeometry& geom) { geom.model = vl::GpuAssetManager->GetGpuHandle(model); });
	}

	if (dirtyFlags[DF::SRT]) {
		Enqueue([trans = GetNodeTransformWCS()](SceneGeometry& geom) { geom.transform = trans; });
	}
}

GeometryNode::~GeometryNode()
{
	Scene->EnqueueDestroyCmd<SceneGeometry>(sceneUid);
}
