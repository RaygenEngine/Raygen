#include "world/nodes/geometry/GeometryNode.h"
#include "asset/AssetManager.h"

void ::GeometryNode::DirtyUpdate(DirtyFlagset dirtyFlags)
{
	if (dirtyFlags[DF::ModelChange]) {
		SetLocalBB(m_model.Lock()->bbox);
	}
}
