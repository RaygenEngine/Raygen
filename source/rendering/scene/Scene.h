#pragma once
#include "rendering/scene/SceneCollection.h"

#include <mutex>

struct Scene {

	size_t activeCamera{ 0 };

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

	~Scene();

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
