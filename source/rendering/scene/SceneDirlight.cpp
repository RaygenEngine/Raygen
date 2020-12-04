#include "SceneDirlight.h"

#include "core/math-ext/Frustum.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/util/WriteDescriptorSets.h"

void SceneDirlight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	auto& extent = shadowmapPass.at(0).framebuffer.extent;

	if (width != extent.width || height != extent.height) {

		for (size_t i = 0; i < c_framesInFlight; ++i) {
			// TODO: this should be called once at the start (constructor - tidy scene structs)
			shadowmapDescSet[i] = vl::Layouts->singleSamplerDescLayout.AllocDescriptorSet();

			shadowmapPass[i] = vl::Layouts->shadowPassLayout.CreatePassInstance(width, height);

			depthSampler = vl::GpuAssetManager->GetShadow2dSampler();

			rvk::writeDescriptorImages(
				shadowmapDescSet[i], 0u, { shadowmapPass[i].framebuffer[0].view() }, depthSampler);
		}
	}
}

void SceneDirlight::UpdateBox(const math::Frustum& frustum, glm::vec3 apex)
{
	glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f) + glm::vec3(ubo.front), up);
	auto aabb = frustum.FrustumPyramidAABB(apex);
	aabb = aabb.Transform(view);

	glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, aabb.min.z, aabb.max.z);

	const float scaleX = 2.0f / (aabb.max.x - aabb.min.x);
	const float scaleY = 2.0f / (aabb.max.y - aabb.min.y);
	const float offsetX = -0.5f * (aabb.min.x + aabb.max.x) * scaleX;
	const float offsetY = -0.5f * (aabb.min.y + aabb.max.y) * scaleY;

	glm::mat4 cropMatrix(1.0f);
	cropMatrix[0][0] = scaleX;
	cropMatrix[1][1] = scaleY;
	cropMatrix[3][0] = offsetX;
	cropMatrix[3][1] = offsetY;

	// NOTE: use scene top level AABB
	// Four of the planes of the orthographic light frustum were calculated using the minimum and maximum of the X and Y
	// coordinates of the view frustum in light space. The last two planes of the orthogonal view frustum are the near
	// and the far planes. To find these planes, the scene's bounds are clipped against the four known light frustum
	// planes. The smallest and largest Z-values from the newly clipped boundary represent the near plane and far plane,
	// respectively. ubo.viewProj = cropMatrix * proj * view;
}
