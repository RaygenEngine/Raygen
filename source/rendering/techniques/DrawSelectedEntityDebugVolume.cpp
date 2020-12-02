#include "DrawSelectedEntityDebugVolume.h"

#include "editor/Editor.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/VolumePipe.h"

namespace vl {

void DrawSelectedEntityDebugVolume::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	auto selEnt = Editor::GetSelection();

	if (selEnt) {
		// StaticPipes::Get<VolumePointsPipe>().Draw(cmdBuffer, sceneDesc, selEnt);
		StaticPipes::Get<VolumeLinesPipe>().Draw(cmdBuffer, sceneDesc, selEnt);
		StaticPipes::Get<VolumeTrianglesPipe>().Draw(cmdBuffer, sceneDesc, selEnt);
	}
}

} // namespace vl
