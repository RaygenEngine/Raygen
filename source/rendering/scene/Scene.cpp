#include "pch.h"
#include "Scene.h"

#include "assets/shared/GeometryShared.h"
#include "engine/console/ConsoleVariable.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneReflectionProbe.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/scene/ScenePointlight.h"

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

Scene::Scene()
{
	EnqueueEndFrame();
	EnqueueCreateCmd<SceneCamera>(); // TODO: editor camera

	sceneAsDescSet = vl::Layouts->accelLayout.AllocDescriptorSet();
}

Scene::~Scene()
{
	vl::Device->waitIdle();
	DrainQueueForDestruction();
	auto destroyVec = [](auto& vec) {
		for (auto el : vec) {
			delete el;
		}
	};

	destroyVec(geometries.elements);
	destroyVec(animatedGeometries.elements);
	destroyVec(cameras.elements);
	destroyVec(spotlights.elements);
	destroyVec(pointlights.elements);
	destroyVec(directionalLights.elements);
	destroyVec(reflProbs.elements);
}

void Scene::UpdateTopLevelAs()
{
	// WIP:
	tlas = vl::TopLevelAs(geometries.elements, this);

	std::array accelStructs{ tlas.handle() };

	vk::WriteDescriptorSetAccelerationStructureKHR descASInfo{};
	descASInfo.setAccelerationStructures(accelStructs);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(sceneAsDescSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR)
		.setDescriptorCount(1u)
		.setPNext(&descASInfo);

	// single call to update all descriptor sets with the new depth image
	vl::Device->updateDescriptorSets(descriptorWrite, {});

	vl::Device->waitIdle();
}

ConsoleVariable<int32> cons_sceneUpdateRt{ "rt.minFrames", 10,
	"Min frames to do progressive before reseting due to scene update" };

void Scene::UploadDirty(uint32 frameIndex)
{
	const bool primaryDirty = activeCamera > 0 && cameras.elements[activeCamera]->isDirty[frameIndex];
	bool anyDirty = false;

	bool requireUpdateAccel = false || forceUpdateAccel;
	for (auto gm : geometries.elements) {
		if (gm && gm->isDirty[frameIndex]) {
			requireUpdateAccel = true;
			gm->isDirty = false;
			anyDirty = true;
		}
	}


	for (auto cam : cameras.elements) {
		if (cam && cam->isDirty[frameIndex]) {
			cam->UploadUbo(frameIndex);
			cam->isDirty[frameIndex] = false;
			anyDirty = true;
		}
	}

	for (auto sl : spotlights.elements) {
		if (sl && sl->isDirty[frameIndex]) {
			sl->UploadUbo(frameIndex);
			sl->isDirty[frameIndex] = false;
			requireUpdateAccel = true;
			anyDirty = true;
		}
	}

	for (auto pl : pointlights.elements) {
		if (pl && pl->isDirty[frameIndex]) {
			pl->UploadUbo(frameIndex);
			pl->isDirty[frameIndex] = false;
			requireUpdateAccel = true;
			anyDirty = true;
		}
	}

	if (requireUpdateAccel) {
		UpdateTopLevelAs();
	}

	for (auto dl : directionalLights.elements) {

		if (dl && primaryDirty) {
			dl->UpdateBox(cameras.elements[activeCamera]->frustum, cameras.elements[activeCamera]->ubo.position);
		}

		if (dl && dl->isDirty[frameIndex]) {

			dl->UploadUbo(frameIndex);
			dl->isDirty[frameIndex] = false;
			anyDirty = true;
		}
	}

	for (auto an : animatedGeometries.elements) {
		if (an && an->isDirtyResize[frameIndex]) {
			an->ResizeJoints(frameIndex);
			an->isDirtyResize[frameIndex] = false;
		}

		if (an && an->isDirty[frameIndex]) {
			an->UploadSsbo(frameIndex);
			an->isDirty[frameIndex] = false;
		}
	}

	if (anyDirty && vl::Renderer->m_raytracingPass.m_rtFrame >= *cons_sceneUpdateRt) {
		vl::Renderer->m_raytracingPass.m_rtFrame = 0;
	}

	// for (auto rp : reflProbs.elements) {
	//	if (rp && rp->isDirty[frameIndex]) {
	//		rp->UploadUbo(frameIndex);
	//		rp->isDirty[frameIndex] = false;
	//	}
	//}
}
