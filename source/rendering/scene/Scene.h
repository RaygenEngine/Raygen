#pragma once
// MAINT: Remove
#include "rendering/Device.h"
#include "rendering/wrappers/TopLevelAs.h"

#include <mutex>
#include <stack>

struct SceneCamera;

template<typename T>
concept CSceneElem = true;

struct SceneCollectionBase {
	virtual ~SceneCollectionBase() = default;

protected:
	friend struct Scene;

	// Typeless pointers to elements.
	std::vector<void*> elements;

	size_t elementResize{ 0 };
	std::vector<size_t> condensedLocation;

	void UpdateElementSize()
	{
		elements.resize(elementResize);
		condensedLocation.resize(elementResize);
	}
};

//
template<CSceneElem T>
struct SceneCollection : public SceneCollectionBase {
	// provides sequential access to the valid elements with unspecified order (non uid order)
	auto begin() const { return condensed.cbegin(); }
	auto end() const { return condensed.cend(); }

private:
	friend struct Scene;
	friend struct vl::TopLevelAs; // TODO: remove this when toplevelas is refactored


	T* Get(size_t uid) { return reinterpret_cast<T*>(elements[uid]); }
	void Set(size_t uid, T* value) { elements[uid] = reinterpret_cast<void*>(value); }


	// condensed vector of pointers (ie no gaps. order is unspecified but preserved internally to allow deletion)
	std::vector<T*> condensed;
	std::vector<size_t> condensedToUid;


	// UID manager section (game thread)
	std::stack<size_t> gaps;
	size_t nextUid{ 0 };

	// Game Thread
	size_t GetNextUid()
	{
		if (gaps.empty()) {
			auto uid = nextUid;
			elementResize = ++nextUid;
			return uid;
		}
		auto top = gaps.top();
		gaps.pop();
		return top;
	}

	// Game Thread
	void RemoveUid(size_t uid) { gaps.push(uid); }

	// Scene Thread
	// Efficiently removes a uid from the condensed vector & updates the relevant condensedLocation information
	void SwapPopFromCondensed(size_t Uid)
	{
		size_t cIndex = condensedLocation[Uid];
		size_t backUid = condensedToUid.back();

		condensed[cIndex] = condensed.back();
		condensed.pop_back();

		condensedToUid[cIndex] = backUid;
		condensedToUid.pop_back();

		condensedLocation[backUid] = cIndex;
	}

	// NEXT: Correct destructor
};


struct Scene {
	std::unordered_map<size_t, UniquePtr<SceneCollectionBase>> collections;

	template<CSceneElem T>
	SceneCollection<T>& Get()
	{
		return *static_cast<SceneCollection<T>*>(collections.at(mti::GetHash<T>()).get());
	}


private:
	void ExecuteCreations()
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		for (auto& [hash, collection] : collections) {
			collection->UpdateElementSize();
		}
	}

public:
	vl::TopLevelAs tlas;

	bool forceUpdateAccel; // NEXT: Remove

	std::vector<UniquePtr<std::vector<std::function<void()>>>> cmds;
	std::vector<std::function<void()>>* currentCmdBuffer;

	std::mutex cmdBuffersVectorMutex;
	std::mutex cmdAddPendingElementsMutex;

	size_t activeCamera{ 0 };

	vk::UniqueAccelerationStructureKHR sceneAS;
	vk::DescriptorSet sceneAsDescSet;

	template<CSceneElem T>
	T* GetElement(size_t uid)
	{
		return Get<T>().Get(uid);
	}

	size_t ctxHash;
	SceneCollectionBase* ctxCollection;


	// Prepare the context (scene class type) for the following "InCtx" commands
	template<CSceneElem T>
	void SetCtx()
	{
		ctxHash = mti::GetHash<T>();
		ctxCollection = collections[ctxHash].get();
	}

	template<CSceneElem T>
	void EnqueueCmdInCtx(size_t uid, std::function<void(T&)>&& command)
	{
		CLOG_ABORT(ctxHash != mti::GetHash<T>(),
			"EnqueueCmdInCtx was called with incorrect context. Use SetCtx before this call or EnqueueCmd if you only "
			"want a single cmd (slower per call).");

		currentCmdBuffer->emplace_back(
			[&, cmd = std::move(command), collection = static_cast<SceneCollection<T>*>(ctxCollection), uid]() {
				T& sceneElement = *collection->Get(uid);
				cmd(sceneElement);
				auto& dirtyVec = sceneElement.isDirty;
				std::fill(dirtyVec.begin(), dirtyVec.end(), true);
			});
	}

	template<CSceneElem T>
	void EnqueueCmd(size_t uid, std::function<void(T&)>&& command)
	{
		currentCmdBuffer->emplace_back([&, cmd = std::move(command), uid]() {
			T& sceneElement = *Get<T>().Get(uid);
			cmd(sceneElement);
			auto& dirtyVec = sceneElement.isDirty;
			std::fill(dirtyVec.begin(), dirtyVec.end(), true);
		});
	}

	// "Dumb" interface, allows non linear allocation of uids later (for filling holes)
	// outCreations modifies the given uids given by the pointers
	template<CSceneElem T>
	void EnqueueCreateDestoryCmds(std::vector<size_t>&& destructions, std::vector<size_t*>&& outCreations)
	{
		if (destructions.empty() && outCreations.empty()) {
			return;
		}

		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		SceneCollection<T>& type = Get<T>();
		for (auto& uid : destructions) {
			type.RemoveUid(uid);
		}

		std::vector<size_t> constructions;

		for (auto& scUid : outCreations) {
			size_t uid = type.GetNextUid();
			*scUid = uid;
			constructions.emplace_back(uid);
		}

		currentCmdBuffer->emplace_back([&, destr = std::move(destructions), constr = std::move(constructions)]() {
			// TODO: deferred deleting of scene objects
			vl::Device->waitIdle();
			for (auto uid : destr) {
				delete type.Get(uid);
				type.Set(uid, nullptr);

				// Condensing
				// Swap & Pop using condensedLocation to locate elements
				type.SwapPopFromCondensed(uid);
			}

			for (auto uid : constr) {
				type.Set(uid, new T());
				// Condensing
				type.condensed.emplace_back(type.Get(uid));
				type.condensedToUid.emplace_back(uid);
				type.condensedLocation[uid] = type.condensed.size() - 1;
			}
		});
	}

	void EnqueueEndFrame()
	{
		// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
		currentCmdBuffer = cmds.emplace_back(std::make_unique<std::vector<std::function<void()>>>()).get();
	}

	void EnqueueActiveCameraCmd(size_t uid)
	{
		currentCmdBuffer->emplace_back([&, uid]() { activeCamera = uid; });
	}


public:
	void BuildAll();

	void ConsumeCmdQueue()
	{
		ExecuteCreations();

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
