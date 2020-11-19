#include "IrradianceGridComponent.h"

#include "rendering/scene/SceneIrradianceGrid.h"

DECLARE_DIRTY_FUNC(CIrradianceGrid)(BasicComponent& bc)
{
	auto build = notifyBuild;
	notifyBuild = false;

	return [=, position = bc.world().position](SceneIrradianceGrid& ig) {
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
