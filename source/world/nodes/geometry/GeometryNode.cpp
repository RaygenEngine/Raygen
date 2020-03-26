#include "pch.h"
#include "world/nodes/geometry/GeometryNode.h"

#include "renderer/asset/GpuAssetManager.h"
#include "assets/AssetManager.h"
#include "renderer/scene/Scene.h"

GeometryNode::GeometryNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneGeometry>();
}

void GeometryNode::SetModel(PodHandle<ModelPod> newModel)
{
	m_model = newModel;
	SetDirty(DF::ModelChange);
}

void GeometryNode::DirtyUpdate(DirtyFlagset dirtyFlags)
{
	Node::DirtyUpdate(dirtyFlags);

	if (dirtyFlags[DF::ModelChange]) {
		m_localBB = m_model.Lock()->bbox;
		CalculateWorldAABB();

		Enqueue([model = GetModel()](SceneGeometry& geom) { geom.model = GpuAssetManager.GetGpuHandle(model); });
	}

	if (dirtyFlags[DF::SRT]) {
		Enqueue([trans = GetNodeTransformWCS()](SceneGeometry& geom) { geom.transform = trans; });
	}
}

GeometryNode::~GeometryNode()
{
	Scene->EnqueueDestroyCmd<SceneGeometry>(sceneUid);
}
