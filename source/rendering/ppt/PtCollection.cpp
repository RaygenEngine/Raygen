#include "pch.h"
#include "PtCollection.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/ppt/lightpass/PtSpotlight.h"
#include "rendering/ppt/lightpass/PtReflProb.h"

namespace vl {

// Keep the registration function at the top of the file
void PtCollection::RegisterTechniques()
{
	//
	// All postproc techniques should be registered here with the proper rendering order.
	//

	// LIGHT PASS
	NextTechnique<PtSpotlight>();
	NextTechnique<PtReflProb>();

	//
	RunPrepares();
}

void PtCollection::RunPrepares()
{
	for (auto& entry : m_postprocTechs) {
		entry.instance->Prepare();
	}
}

void PtCollection::Draw(vk::CommandBuffer buffer, uint32 frameIndex)
{
	// Probably pointless to not draw anything, remove when in editor settings are available
	static ConsoleVariable<bool> console_drawPostProc{ "r.drawPostProc", true,
		"Wether to draw any post process pass from pt collection. (includes lights)" };

	if (!console_drawPostProc.Get()) {
		return;
	}

	for (auto& entry : m_postprocTechs) {
		[[unlikely]] if (!entry.isEnabled) { continue; }

		entry.instance->Draw(buffer, frameIndex);
	}
}
} // namespace vl
