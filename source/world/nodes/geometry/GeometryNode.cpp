#include "world/nodes/geometry/GeometryNode.h"
#include "asset/AssetManager.h"
#include "core/MathAux.h"

void GeometryNode::DirtyUpdate(DirtyFlagset dirtyFlags)
{
	if (dirtyFlags[DF::ModelChange]) {
		m_localBB = m_model.Lock()->bbox;
	}
	else if (dirtyFlags[DF::TRS]) {
		// calculate aabb
		m_aabb = math::TransformedAABB(m_localBB, GetWorldMatrix());
	}
}
