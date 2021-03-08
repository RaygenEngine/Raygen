#include "Scene.h"

#include "engine/console/ConsoleVariable.h"
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
	// currentCmdBuffer->emplace_back([&, uid]() { activeCamera = uid; });
}

void Scene::BuildAll()
{
	// for (auto reflProb : Get<SceneReflprobe>()) {
	// reflProb->Build();
	//}
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
}

Scene::~Scene()
{
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

ConsoleVariable<int32> cons_sceneUpdateRt{ "rt.minFrames", 10,
	"Min frames to do progressive before reseting due to scene update" };

// NEW::
void Scene::UploadDirty(uint32 frameIndex)
{
	// bool requireUpdateAccel = false || forceUpdateAccel;
	// for (auto gm : Get<SceneGeometry>()) {
	//	gm->prevTransform = gm->transform;
	//	if (gm->isDirty[frameIndex]) {
	//		requireUpdateAccel = true;
	//		gm->isDirty = false;
	//	}
	//}


	// for (auto cam : Get<SceneCamera>()) {
	//	if (cam->isDirty[frameIndex]) {
	//		cam->UploadUbo(frameIndex);
	//		cam->isDirty[frameIndex] = false;
	//	}
	//}

	// for (auto ig : Get<SceneIrragrid>()) {
	//	if (ig->isDirty[frameIndex]) {
	//		ig->UploadUbo(frameIndex);
	//		requireUpdateAccel = true;
	//		ig->isDirty[frameIndex] = false;
	//	}
	//}

	// for (auto rp : Get<SceneReflprobe>()) {
	//	if (rp->isDirty[frameIndex]) {
	//		rp->UploadUbo(frameIndex);
	//		requireUpdateAccel = true;
	//		rp->isDirty[frameIndex] = false;
	//	}
	//}

	// for (auto sl : Get<SceneSpotlight>()) {
	//	if (sl->isDirty[frameIndex]) {
	//		sl->UploadUbo(frameIndex);
	//		sl->isDirty[frameIndex] = false;
	//		requireUpdateAccel = true;
	//	}
	//}

	// for (auto pl : Get<ScenePointlight>()) {
	//	if (pl->isDirty[frameIndex]) {
	//		pl->UploadUbo(frameIndex);
	//		pl->isDirty[frameIndex] = false;
	//		requireUpdateAccel = true;
	//	}
	//}

	// for (auto ql : Get<SceneQuadlight>()) {
	//	if (ql->isDirty[frameIndex]) {
	//		ql->UploadUbo(frameIndex);
	//		ql->isDirty[frameIndex] = false;
	//		requireUpdateAccel = true;
	//	}
	//}

	// for (auto dl : Get<SceneDirlight>()) {
	//	if (dl->isDirty[frameIndex]) {
	//		dl->UploadUbo(frameIndex);
	//		dl->isDirty[frameIndex] = false;
	//		requireUpdateAccel = true;
	//	}
	//}

	// for (auto an : Get<SceneAnimatedGeometry>()) {
	//	if (an->isDirtyResize[frameIndex]) {
	//		an->ResizeJoints(frameIndex);
	//		an->isDirtyResize[frameIndex] = false;
	//	}

	//	if (an->isDirty[frameIndex]) {
	//		an->UploadSsbo(frameIndex);
	//		an->isDirty[frameIndex] = false;
	//	}
	//}

	// if (requireUpdateAccel) {
	//	UpdateTopLevelAs();
	//}
}
