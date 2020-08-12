#pragma once
#include "rendering/ppt/PtEntry.h"
#include "rendering/scene/Scene.h"

namespace vl {
class PtCollection {
	std::vector<PtEntry> m_postprocTechs;

	template<CONC(CPostTech) T>
	void NextTechnique()
	{
		PtEntry e{
			.instance = std::make_unique<T>(),    //
			.type = mti::GetTypeId<T>(),          //
			.entryIndex = m_postprocTechs.size(), //
			.constructor =
				[]() {
					return new T();
				}
			//
		};
		m_postprocTechs.push_back(std::move(e));
	}

	void RunPrepares();

public:
	void Draw(vk::CommandBuffer buffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet gbufferDescSet);
	void RegisterTechniques();
};
} // namespace vl
