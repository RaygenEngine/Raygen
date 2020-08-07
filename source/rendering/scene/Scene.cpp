#include "pch.h"
#include "Scene.h"

#include "assets/shared/GeometryShared.h"
#include "engine/console/ConsoleVariable.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneReflectionProbe.h"
#include "rendering/scene/SceneSpotlight.h"


void Scene::BuildAll()
{
	for (auto reflProb : reflProbs.elements) {
		reflProb->Build();
	}
}

void Scene::DrainQueueForDestruction()
{
	ExecuteCreations();

	size_t begin = 0;
	size_t end = cmds.size();

	if (end <= 0) {
		return;
	}

	for (size_t i = 0; i < end; ++i) {
		for (auto& cmd : *cmds[i]) {
			cmd();
		}
	}

	// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
	cmds.erase(cmds.begin(), cmds.begin() + end);
}

Scene::Scene(size_t size)
	: size(size)
{
	EnqueueEndFrame();
	EnqueueCreateCmd<SceneCamera>();
}

vk::DescriptorSet Scene::GetActiveCameraDescSet()
{
	return GetActiveCamera()->descSets[vl::Renderer_::currentFrame];
}

void Scene::UploadDirty()
{
	const bool primaryDirty = activeCamera > 0 && cameras.elements[activeCamera]->isDirty[vl::Renderer_::currentFrame];

	for (auto cam : cameras.elements) {
		if (cam && cam->isDirty[vl::Renderer_::currentFrame]) {
			cam->UploadUbo(vl::Renderer_::currentFrame);
			cam->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto sl : spotlights.elements) {
		if (sl && sl->isDirty[vl::Renderer_::currentFrame]) {
			sl->UploadUbo(vl::Renderer_::currentFrame);
			sl->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto dl : directionalLights.elements) {

		if (dl && primaryDirty) {
			dl->UpdateBox(cameras.elements[activeCamera]->frustum, cameras.elements[activeCamera]->ubo.position);
		}

		if (dl && dl->isDirty[vl::Renderer_::currentFrame]) {

			dl->UploadUbo(vl::Renderer_::currentFrame);
			dl->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto an : animatedGeometries.elements) {
		if (an && an->isDirtyResize[vl::Renderer_::currentFrame]) {
			an->ResizeJoints(vl::Renderer_::currentFrame);
			an->isDirtyResize[vl::Renderer_::currentFrame] = false;
		}

		if (an && an->isDirty[vl::Renderer_::currentFrame]) {
			an->UploadSsbo(vl::Renderer_::currentFrame);
			an->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	// for (auto rp : reflProbs.elements) {
	//	if (rp && rp->isDirty[vl::Renderer_::currentFrame]) {
	//		rp->UploadUbo(vl::Renderer_::currentFrame);
	//		rp->isDirty[vl::Renderer_::currentFrame] = false;
	//	}
	//}
}
