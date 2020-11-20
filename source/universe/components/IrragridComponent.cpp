#include "IrragridComponent.h"

#include "rendering/scene/SceneIrragrid.h"

DECLARE_DIRTY_FUNC(CIrragrid)(BasicComponent& bc)
{
	auto build = notifyBuild;
	notifyBuild = false;

	return [=, position = bc.world().position](SceneIrragrid& ig) {
		ig.ubo.posAndDist = glm::vec4(position, distToAdjacent);

		if (FullDirty) {
			ig.ubo.width = width;
			ig.ubo.height = height;
			ig.ubo.depth = depth;
			ig.resolution = resolution;

			ig.ptSamples = ptSamples;
			ig.ptBounces = ptBounces;
			if (build) {
				// PERF:
				ig.Allocate();
			}
		}
	};
}
