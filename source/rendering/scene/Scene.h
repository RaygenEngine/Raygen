#pragma once
// MAINT: Remove
#include "rendering/Device.h"
#include "rendering/wrappers/TopLevelAs.h"
#include "rendering/scene/SceneCollection.h"

#include <mutex>

struct SceneCamera;
struct Scene;
struct SceneRenderDesc {
	Scene* scene{ nullptr };
	SceneCamera& viewer;

	uint32 frameIndex{ 0 };

	vk::DescriptorSet attachmentsDescSet;

	// TODO: Scene description should only contain the required scene structs for current frame rendering
	// apart from occlusion etc
	SceneRenderDesc(Scene* scene_, SceneCamera& viewer_, uint32 frameIndex)
		: scene(scene_)
		, viewer(viewer_)
		, frameIndex(frameIndex)
	{
	}

	Scene* operator->() const { return scene; }
};

struct Scene {

public:
	vl::TopLevelAs tlas;

	bool forceUpdateAccel{ true }; // NEXT: Remove

	size_t activeCamera{ 0 };

	vk::UniqueAccelerationStructureKHR sceneAS;
	vk::DescriptorSet sceneAsDescSet;

public:
	// Get a collection of the specific Scene Class type. Iterating [begin(), end()] will only return valid subobjects
	template<CSceneElem T>
	SceneCollection<T>& Get();

	template<CSceneElem T>
	T* GetElement(size_t uid);

	// Prepare the context (scene class type) for the following "InCtx" commands
	template<CSceneElem T>
	void SetCtx();

	template<CSceneElem T>
	void EnqueueCmdInCtx(size_t uid, std::function<void(T&)>&& command);

	template<CSceneElem T>
	void EnqueueCmd(size_t uid, std::function<void(T&)>&& command);


	// "Dumb" interface, allows non linear allocation of uids (for filling holes)
	// outCreations modifies the given uids given by the pointers
	template<CSceneElem T>
	void EnqueueCreateDestoryCmds(std::vector<size_t>&& destructions, std::vector<size_t*>&& outCreations);

	void EnqueueEndFrame();
	void EnqueueActiveCameraCmd(size_t uid);

	void BuildAll();
	void ConsumeCmdQueue();

	// CHECK: usage? duplicate code?
	void DrainQueueForDestruction();

	// CHECK: runs 2 frames behind
	Scene();

	void UploadDirty(uint32 frameIndex);

	SceneRenderDesc GetRenderDesc(int32 frameIndex);

	~Scene();
	void UpdateTopLevelAs();

	Scene(Scene const&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene const&) = delete;
	Scene& operator=(Scene&&) = delete;

private:
	std::unordered_map<size_t, UniquePtr<SceneCollectionBase>> collections;

	std::vector<UniquePtr<std::vector<std::function<void()>>>> cmds;
	std::vector<std::function<void()>>* currentCmdBuffer;

	std::mutex cmdBuffersVectorMutex;
	std::mutex cmdAddPendingElementsMutex;

	size_t ctxHash;
	SceneCollectionBase* ctxCollection;
};

#include "Scene.impl.h"
