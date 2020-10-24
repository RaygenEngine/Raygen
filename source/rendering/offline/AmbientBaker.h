#pragma once

#include "rendering/offline/IrradianceMapCalculation.h"
#include "rendering/offline/PathtracedCubemap.h"
#include "rendering/offline/PrefilteredMapCalculation.h"
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {
class AmbientBaker {
public:
	AmbientBaker(SceneReflprobe* rp);

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void Resize(uint32 resolution);

	RCubemap m_surroundingEnv;
	RCubemap m_irradiance;
	RCubemap m_prefiltered;

private:
	PathtracedCubemap m_ptCubemap;
	IrradianceMapCalculation m_irrCalc;
	PrefilteredMapCalculation m_prefCalc;


	vk::DescriptorSet m_surroundingEnvStorageDescSet;
	vk::DescriptorSet m_surroundingEnvSamplerDescSet;
};
} // namespace vl
