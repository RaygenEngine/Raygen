#include "pch/pch.h"

#include "world/nodes/light/DirectionalLightNode.h"

void DirectionalLightNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Projection]) {
		UpdateProjectionMatrix();
	}
}
