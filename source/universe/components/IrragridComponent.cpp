#include "IrragridComponent.h"

#include "rendering/scene/SceneIrragrid.h"

DECLARE_DIRTY_FUNC(CIrragrid)(BasicComponent& bc)
{
	return [=, translation = bc.world().translation()](SceneIrragrid& ig) {
		XMStoreFloat4A(&ig.ubo.posAndDist, translation);
		ig.ubo.posAndDist.w = distToAdjacent;
		if (FullDirty) {
			ig.ubo.width = width;
			ig.ubo.height = height;
			ig.ubo.depth = depth;
			ig.irrResolution = irrResolution;

			ig.ptSamples = ptSamples;
			ig.ptBounces = ptBounces;
		}
	};
}
