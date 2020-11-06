#include "IrradianceGridComponent.h"

#include "rendering/scene/SceneIrradianceGrid.h"

DECLARE_DIRTY_FUNC(CIrradianceGrid)(BasicComponent& bc)
{
	auto build = notifyBuild;
	notifyBuild = false;

	return [=, position = bc.world().position](SceneIrradianceGrid& ig) {
		if (FullDirty) {
			ig.distToAdjacent = distToAdjacent;
			ig.blendProportion = blendProportion;
			ig.shouldBuild.Assign(build);
		}

		ig.ShouldRecalculatePositions(position);
	};
}
