#include "pch.h"
#include "PtCollection.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/ppt/lightpass/PtSpotlight.h"
#include "rendering/ppt/lightpass/PtReflProb.h"
#include "rendering/ppt/lightpass/PtDirectionalLight.h"
#include "rendering/ppt/lightpass/PtPointlight.h"
#include "rendering/ppt/techniques/PtDebug.h"


namespace vl {

// Keep the registration function at the top of the file
void PtCollection::RegisterTechniques()
{
	//
	// All postproc techniques should be registered here with the proper rendering order.
	//

	// LIGHT PASS
	NextTechnique<PtDirectionalLight>();
	NextTechnique<PtSpotlight>();
	NextTechnique<PtPointlight>();
	NextTechnique<PtReflProb>();


	// COLOR

	NextTechnique<PtDebug>();


	//
	RunPrepares();
}

void PtCollection::RunPrepares()
{
	for (auto& entry : m_postprocTechs) {
		entry.instance->Prepare();
	}
}

void PtCollection::Draw(vk::CommandBuffer buffers, const SceneRenderDesc& sceneDesc)
{
	// Probably pointless to not draw anything, remove when in editor settings are available
	static ConsoleVariable<bool> console_drawPostProc{ "r.drawPostProc", true,
		"Wether to draw any post process pass from pt collection. (includes lights)" };

	if (!console_drawPostProc.Get()) {
		return;
	}


	for (auto& entry : m_postprocTechs) {
		if (!entry.isEnabled)
			[[unlikely]] { continue; }

		entry.instance->Draw(buffers, sceneDesc);
	}
}
} // namespace vl
