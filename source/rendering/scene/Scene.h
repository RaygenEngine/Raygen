#pragma once
// MAINT: Remove
#include "rendering/Device.h"
#include "rendering/wrappers/TopLevelAs.h"

#include <mutex>
#include <stack>

struct SceneGeometry;
struct SceneCamera;
struct SceneSpotlight;
struct SceneDirlight;
struct SceneReflProbe;
struct SceneAnimatedGeometry;
struct ScenePointlight;


template<typename T>
concept CSceneElem
	= std::is_same_v<SceneGeometry,
		  T> || std::is_same_v<SceneCamera, T> || std::is_same_v<ScenePointlight, T> || std::is_same_v<SceneSpotlight, T> || std::is_same_v<SceneDirlight, T> || std::is_same_v<SceneReflProbe, T> || std::is_same_v<SceneAnimatedGeometry, T>;

template<CSceneElem T>
struct SceneVector {
	std::vector<T*> elements;
	size_t elementResize{ 0 };

	void AppendPendingElements() { elements.resize(elementResize); }


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


	// NEXT: Correct destructor
};

struct Scene {
	SceneVector<SceneGeometry> geometries;
	SceneVector<SceneAnimatedGeometry> animatedGeometries;
	SceneVector<SceneCamera> cameras;
	SceneVector<SceneSpotlight> spotlights;
	SceneVector<ScenePointlight> pointlights;
	SceneVector<SceneDirlight> directionalLights;
	SceneVector<SceneReflProbe> reflProbs;

	template<CSceneElem T>
	SceneVector<T>& GetType()
	{
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			return geometries;
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			return cameras;
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			return spotlights;
		}
		else if constexpr (std::is_same_v<ScenePointlight, T>) {
			return pointlights;
		}
		else if constexpr (std::is_same_v<SceneDirlight, T>) {
			return directionalLights;
		}
		else if constexpr (std::is_same_v<SceneReflProbe, T>) {
			return reflProbs;
		}
		else if constexpr (std::is_same_v<SceneAnimatedGeometry, T>) {
			return animatedGeometries;
		}
		LOG_ABORT("Incorrect type");
	}

private:
	void ExecuteCreations()
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		geometries.AppendPendingElements();
		cameras.AppendPendingElements();
		spotlights.AppendPendingElements();
		pointlights.AppendPendingElements();
		directionalLights.AppendPendingElements();
		reflProbs.AppendPendingElements();
		animatedGeometries.AppendPendingElements();
	}

public:
	vl::TopLevelAs tlas;

	bool forceUpdateAccel; // NEXT: Remove

	std::vector<UniquePtr<std::vector<std::function<void()>>>> cmds;
	std::vector<std::function<void()>>* currentCmdBuffer;

	std::mutex cmdBuffersVectorMutex;
	std::mutex cmdAddPendingElementsMutex;

	size_t activeCamera{ 0 };
	// size_t size{ 0 };

	vk::UniqueAccelerationStructureKHR sceneAS;
	vk::DescriptorSet sceneAsDescSet;

	template<CSceneElem T>
	T*& GetElement(size_t uid)
	{
		return GetType<T>().elements[uid];
	}

	template<CSceneElem T>
	void EnqueueCmd(size_t uid, std::function<void(T&)>&& command)
	{
		currentCmdBuffer->emplace_back([&, cmd = std::move(command), uid]() {
			//
			cmd(*GetElement<T>(uid));
			auto& dirtyVec = GetElement<T>(uid)->isDirty;
			std::fill(dirtyVec.begin(), dirtyVec.end(), true);
		});
	}

	// "Dumb" interface, will allow non linear allocation of uids later (for filling holes)
	template<CSceneElem T>
	void EnqueueCreateDestoryCmds(std::vector<size_t>&& destructions, std::vector<size_t*>&& outCreations)
	{
		if (destructions.empty() && outCreations.empty()) {
			return;
		}

		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		SceneVector<T>& type = GetType<T>();
		for (auto& uid : destructions) {
			type.RemoveUid(uid);
		}

		std::vector<size_t> constructions;
		size_t elementsSize = type.elements.size();

		for (auto& scUid : outCreations) {
			size_t uid = type.GetNextUid();
			*scUid = uid;
			constructions.emplace_back(uid);
		}

		currentCmdBuffer->emplace_back([&, destr = std::move(destructions), constr = std::move(constructions)]() {
			// TODO: deferred deleting of scene objects
			vl::Device->waitIdle();
			for (auto uid : destr) {
				auto& ptr = type.elements[uid];
				delete ptr;
				ptr = nullptr;
			}

			for (auto uid : constr) {
				type.elements[uid] = new T();
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
