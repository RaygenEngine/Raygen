#include "pch.h"
#include "SceneDirectionalLight.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"

void SceneDirectionalLight::ResizeShadowmap(uint32 width, uint32 height)
{
	shadowmap = std::make_unique<vl::RDepthmap>(width, height);
}

void SceneDirectionalLight::UpdateBox(math::Frustum frustum, glm::vec3 apex)
{
	// TODO: use OBB

	auto aabb = frustum.FrustumPyramidAABB(apex);

	// view cuboid
	auto center = aabb.GetCenter();

	auto view = glm::lookAt(center, center + ubo.forward.xyz, up);

	auto width = aabb.max.x - aabb.min.x;
	auto height = aabb.max.y - aabb.min.y;
	auto length = aabb.max.z - aabb.min.z;

	auto right = width / 2.f;
	auto left = width / 2.f;

	auto top = height / 2.f;
	auto bottom = height / 2.f;

	float near_{ 0.05f };
	float far_{ 20.0f };

	auto proj = glm::ortho(left, right, bottom, top, near_, far_);

	ubo.viewProj = proj * view;
}
