#pragma once

#include "rendering/VkCoreIncludes.h"

struct SceneRenderDesc;
struct SceneReflprobe;

namespace vl {
struct BakeProbes {

	static void RecordCmd(const SceneRenderDesc& sceneDesc);

private:
	static void BakeEnvironment(const SceneRenderDesc& sceneDesc, const std::vector<vk::UniqueImageView>& faceViews,
		const glm::vec3& probePosition, int32 ptSamples, int32 ptBounces, float offset, const vk::Extent3D& extent);
	static void BakeIrradiance(const std::vector<vk::UniqueImageView>& faceViews,
		vk::DescriptorSet environmentSamplerDescSet, const vk::Extent3D& extent);

	// currently only used by reflprobes
	static void BakePrefiltered(const SceneReflprobe& rp);
};

} // namespace vl
