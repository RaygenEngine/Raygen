#pragma once
// MAINT: Remove
#include "rendering/Device.h"
#include "rendering/wrappers/TopLevelAs.h"
#include "rendering/scene/SceneCollection.h"

#include <mutex>

struct SceneCamera;

struct Scene {

	template<CSceneElem T>
	SceneCollection<T>& Get();

public:
	vl::TopLevelAs tlas;

	bool forceUpdateAccel; // NEXT: Remove

	size_t activeCamera{ 0 };

	vk::UniqueAccelerationStructureKHR sceneAS;
	vk::DescriptorSet sceneAsDescSet;

public:
	template<CSceneElem T>
	T* GetElement(size_t uid);

	// Prepare the context (scene class type) for the following "InCtx" commands
	template<CSceneElem T>
	void SetCtx();

	// WIP: Use this in scene cmd system code
	template<CSceneElem T>
	void EnqueueCmdInCtx(size_t uid, std::function<void(T&)>&& command);

	template<CSceneElem T>
	void EnqueueCmd(size_t uid, std::function<void(T&)>&& command);


	// "Dumb" interface, allows non linear allocation of uids later (for filling holes)
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

struct SceneRenderDesc {
	Scene* scene{ nullptr };
	SceneCamera* viewer{ nullptr };

	uint32 frameIndex{ 0 };

	vk::DescriptorSet attDesc;

	// TODO: Scene description should only contain the required scene structs for current frame rendering
	// apart from occlusion etc
	SceneRenderDesc(Scene* scene_, size_t viewerIndex, uint32 frameIndex)
		: scene(scene_)
		, viewer(scene_->GetElement<SceneCamera>(viewerIndex))
		, frameIndex(frameIndex)
	{
	}

	Scene* operator->() const { return scene; }
};

#include "Scene.impl.h"
