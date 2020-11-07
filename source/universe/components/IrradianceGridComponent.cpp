#include "IrradianceGridComponent.h"

#include "rendering/scene/SceneIrradianceGrid.h"

DECLARE_DIRTY_FUNC(CIrradianceGrid)(BasicComponent& bc)
{
	auto build = notifyBuild;
	notifyBuild = false;

	return [=, position = bc.world().position](SceneIrradianceGrid& ig) {
		ig.pos = glm::vec4(position, 1.f);

		if (FullDirty) {
			ig.distToAdjacent = distToAdjacent;
			ig.ptSamples = ptSamples;
			ig.ptBounces = ptBounces;
			ig.shouldBuild.Assign(build);
		}
	};
}
