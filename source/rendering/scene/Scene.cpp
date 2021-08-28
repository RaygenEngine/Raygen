#include "Scene.h"

#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/scene/ScenePointlight.h"
#include "rendering/scene/SceneQuadlight.h"
#include "rendering/scene/SceneReflprobe.h"
#include "rendering/scene/SceneSpotlight.h"

void Scene::EnqueueEndFrame()
{
	// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
	currentCmdBuffer = cmds.emplace_back(std::make_unique<std::vector<std::function<void()>>>()).get();
}

void Scene::EnqueueActiveCameraCmd(size_t uid)
{
	currentCmdBuffer->emplace_back([&, uid]() { activeCamera = uid; });
}

void Scene::BuildAll()
{
	// for (auto reflProb : Get<SceneReflprobe>()) {
	// reflProb->Build();
	//}

	UpdateTopLevelAs();
}

void Scene::ConsumeCmdQueue()
{
	// Resize elements, might move the vector so we need to use a mutex temporarily
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		for (auto& [hash, collection] : collections) {
			collection->UpdateElementSize();
		}
	}

	size_t begin = 0;
	size_t end = cmds.size() - 1;

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

void Scene::DrainQueueForDestruction()
{
	// Resize elements, might move the vector so we need to use a mutex temporarily
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		for (auto& [hash, collection] : collections) {
			collection->UpdateElementSize();
		}
	}
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

namespace {
template<CSceneElem T>
void RegisterSingle(std::unordered_map<size_t, UniquePtr<SceneCollectionBase>>& collections)
{
	collections[mti::GetHash<T>()] = std::make_unique<SceneCollection<T>>();
}

template<CSceneElem... Rest>
void Register(std::unordered_map<size_t, UniquePtr<SceneCollectionBase>>& collections)
{
	(RegisterSingle<Rest>(collections), ...);
}
} // namespace

Scene::Scene()
{
	Register<SceneGeometry, SceneAnimatedGeometry, SceneCamera, SceneSpotlight, ScenePointlight, SceneDirlight,
		SceneReflprobe, SceneIrragrid, SceneQuadlight>(collections);

	EnqueueEndFrame();
	size_t uid;
	EnqueueCreateDestoryCmds<SceneCamera>({}, { &uid }); // TODO: Editor camera
	sceneAsDescSet = vl::DescriptorLayouts->accelerationStructure.AllocDescriptorSet();

	auto unitRectData = rvk::getUnitRectTriangleListInfo();

	quadlightBlas = vl::BottomLevelAs(*unitRectData.vertexBuffer, unitRectData.vertexCount, *unitRectData.indexBuffer,
		unitRectData.indexCount, vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
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


	destroyVec(Get<SceneGeometry>().condensed);
	destroyVec(Get<SceneAnimatedGeometry>().condensed);
	destroyVec(Get<SceneCamera>().condensed);
	destroyVec(Get<SceneSpotlight>().condensed);
	destroyVec(Get<ScenePointlight>().condensed);
	destroyVec(Get<SceneDirlight>().condensed);
	destroyVec(Get<SceneReflprobe>().condensed);
	destroyVec(Get<SceneIrragrid>().condensed);
	destroyVec(Get<SceneQuadlight>().condensed);

	// CHECK: proper type erased cleanup here.
}

void Scene::UpdateTopLevelAs()
{
	// TODO:
	tlas = vl::TopLevelAs(Get<SceneGeometry>().condensed, Get<SceneQuadlight>().condensed, quadlightBlas, this);

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

SceneRenderDesc Scene::GetRenderDesc(int32 frameIndex)
{
	auto camera = GetElement<SceneCamera>(activeCamera);
	if (!camera) {
		// If you get this error, the primary camera in the world is not setup correctly. (eg: a deleted active camera)
		// This is a bug and should be handled at world code properly.
		LOG_ERROR("Failed to find primary camera for scene. Using last camera for now.");

		CLOG_ABORT(
			Get<SceneCamera>().empty(), "Failed to find any camera. This for now is not handled, aborting execution");

		camera = *(Get<SceneCamera>().end() - 1);
	}

	return SceneRenderDesc{ this, *camera, static_cast<uint32>(frameIndex) };
}

ConsoleVariable<int32> cons_sceneUpdateRt{ "r.raytracing.minFrames", 10,
	"Minimum frames to do progressive before reseting due to scene update" };


void Scene::UploadDirty(uint32 frameIndex)
{
	bool requireUpdateAccel = false || forceUpdateAccel;
	for (auto gm : Get<SceneGeometry>()) {
		gm->prevTransform = gm->transform;
		if (gm->isDirty[frameIndex]) {
			requireUpdateAccel = true;
			gm->isDirty = false;
		}
	}


	for (auto cam : Get<SceneCamera>()) {
		if (cam->isDirty[frameIndex]) {
			cam->UploadUbo(frameIndex);
			cam->isDirty[frameIndex] = false;
		}
	}

	for (auto ig : Get<SceneIrragrid>()) {
		if (ig->isDirty[frameIndex]) {
			ig->UploadUbo(frameIndex);
			requireUpdateAccel = true;
			ig->isDirty[frameIndex] = false;
		}
	}

	for (auto rp : Get<SceneReflprobe>()) {
		if (rp->isDirty[frameIndex]) {
			rp->UploadUbo(frameIndex);
			requireUpdateAccel = true;
			rp->isDirty[frameIndex] = false;
		}
	}

	for (auto sl : Get<SceneSpotlight>()) {
		if (sl->isDirty[frameIndex]) {
			sl->UploadUbo(frameIndex);
			sl->isDirty[frameIndex] = false;
			requireUpdateAccel = true;
		}
	}

	for (auto pl : Get<ScenePointlight>()) {
		if (pl->isDirty[frameIndex]) {
			pl->UploadUbo(frameIndex);
			pl->isDirty[frameIndex] = false;
			requireUpdateAccel = true;
		}
	}

	for (auto ql : Get<SceneQuadlight>()) {
		if (ql->isDirty[frameIndex]) {
			ql->UploadUbo(frameIndex);
			ql->isDirty[frameIndex] = false;
			requireUpdateAccel = true;
		}
	}

	for (auto dl : Get<SceneDirlight>()) {
		if (dl->isDirty[frameIndex]) {
			dl->UploadUbo(frameIndex);
			dl->isDirty[frameIndex] = false;
			requireUpdateAccel = true;
		}
	}

	for (auto an : Get<SceneAnimatedGeometry>()) {
		if (an->isDirtyResize[frameIndex]) {
			an->ResizeJoints(frameIndex);
			an->isDirtyResize[frameIndex] = false;
		}

		if (an->isDirty[frameIndex]) {
			an->UploadSsbo(frameIndex);
			an->isDirty[frameIndex] = false;
		}
	}

	if (requireUpdateAccel) {
		UpdateTopLevelAs();
	}

	// WIP: Probe baking should be part of the scene maybe - on the other hand real time probes should be here
	// BakeProbes::RecordCmd(sceneDesc); // NOTE: Blocking
}
