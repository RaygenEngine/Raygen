#include "CalculateDynamicShadowmaps.h"

#include "rendering/pipes/geometry/DepthmapPipe.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneSpotlight.h"

namespace vl {
void CalculateDynamicShadowmaps::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	COMMAND_SCOPE(cmdBuffer, "CalculateShadowmaps::RecordCmd");

	for (auto sl : sceneDesc->Get<SceneSpotlight>()) {
		if (sl->ubo.hasShadow && sl->isDynamic) {
			sl->shadowmapPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
				//
				DepthmapPipe::RecordCmd(cmdBuffer, sl->ubo.viewProj, sceneDesc);
			});
		}
	}

	for (auto dl : sceneDesc->Get<SceneDirlight>()) {
		if (dl->ubo.hasShadow && dl->isDynamic) {
			dl->shadowmapPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
				//
				DepthmapPipe::RecordCmd(cmdBuffer, dl->ubo.viewProj, sceneDesc);
			});
		}
	}
}
} // namespace vl
