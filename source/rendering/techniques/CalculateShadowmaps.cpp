#include "CalculateShadowmaps.h"

#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/geometry/DepthmapPipe.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneSpotlight.h"

namespace vl {
void CalculateShadowmaps::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	for (auto sl : sceneDesc->Get<SceneSpotlight>()) {
		if (sl->ubo.hasShadow) {
			sl->shadowmapPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
				//
				DepthmapPipe::RecordCmd(cmdBuffer, sl->ubo.viewProj, sceneDesc);
			});
		}
	}

	for (auto dl : sceneDesc->Get<SceneDirlight>()) {
		if (dl->ubo.hasShadow) {
			dl->shadowmapPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
				//
				DepthmapPipe::RecordCmd(cmdBuffer, dl->ubo.viewProj, sceneDesc);
			});
		}
	}
}
} // namespace vl
