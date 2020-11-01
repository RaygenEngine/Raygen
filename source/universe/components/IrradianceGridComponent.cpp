#include "IrradianceGridComponent.h"

#include "rendering/scene/SceneIrradianceGrid.h"

DECLARE_DIRTY_FUNC(CIrradianceGrid)(BasicComponent& bc)
{
	auto build = shouldBuild;
	shouldBuild = false;

	return [=, position = bc.world().position](SceneIrradianceGrid& ig) {
		if (FullDirty) {
			ig.distToAdjacent = distToAdjacent;
			ig.shouldBuild.Assign(build);
		}

		ig.ShouldRecalculatePositions(position);
	};
}
