#include "pch.h"
#include "SceneDirectionalLight.h"

#include "core/math-ext/Frustum.h"

void SceneDirectionalLight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	bool shouldResize = true;
	auto& extent = shadowmap.at(0).framebuffer.extent;
	shouldResize = width != extent.width || height != extent.height;

	for (auto& sm : shadowmap) {
		if (shouldResize) {
			sm = vl::Depthmap{ width, height, name.c_str() };
		}
	}
}

void SceneDirectionalLight::UpdateBox(const math::Frustum& frustum, glm::vec3 apex)
{
	glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f) + glm::vec3(ubo.front), up);
	auto aabb = frustum.FrustumPyramidAABB(apex);
	aabb.Transform(view);

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
