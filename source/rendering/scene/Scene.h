#pragma once
// MAINT: Remove
#include "rendering/Device.h"
#include "rendering/wrappers/TopLevelAs.h"

#include <mutex>

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
	size_t pendingElements{ 0 };


	void AppendPendingElements()
	{
		elements.resize(elements.size() + pendingElements);
		pendingElements = 0;
	}

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
	T* GetElement(size_t uid)
	{
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			return geometries.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			return cameras.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			return spotlights.elements.at(uid);
		}
		else if constexpr (std::is_same_v<ScenePointlight, T>) {
			return pointlights.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneDirlight, T>) {
			return directionalLights.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneReflProbe, T>) {
			return reflProbs.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneAnimatedGeometry, T>) {
			return animatedGeometries.elements.at(uid);
		}
		LOG_ABORT("Incorrect type");
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

	template<CSceneElem T>
	size_t EnqueueCreateCmd()
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);

		size_t uid{};
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			uid = geometries.elements.size() + geometries.pendingElements++;
			currentCmdBuffer->emplace_back([&, uid]() { geometries.elements[uid] = new SceneGeometry(); });
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			uid = cameras.elements.size() + cameras.pendingElements++;
			currentCmdBuffer->emplace_back([&, uid]() { cameras.elements[uid] = new SceneCamera(); });
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			uid = spotlights.elements.size() + spotlights.pendingElements++;
			currentCmdBuffer->emplace_back([&, uid]() { spotlights.elements[uid] = new SceneSpotlight(); });
		}
		else if constexpr (std::is_same_v<ScenePointlight, T>) {
			uid = pointlights.elements.size() + pointlights.pendingElements++;
			currentCmdBuffer->emplace_back([&, uid]() { pointlights.elements[uid] = new ScenePointlight(); });
		}
		else if constexpr (std::is_same_v<SceneDirlight, T>) {
			uid = directionalLights.elements.size() + directionalLights.pendingElements++;
			currentCmdBuffer->emplace_back([&, uid]() { directionalLights.elements[uid] = new SceneDirlight(); });
		}
		else if constexpr (std::is_same_v<SceneReflProbe, T>) {
			uid = reflProbs.elements.size() + reflProbs.pendingElements++;
			currentCmdBuffer->emplace_back([&, uid]() { reflProbs.elements[uid] = new SceneReflProbe(); });
		}
		else if constexpr (std::is_same_v<SceneAnimatedGeometry, T>) {
			uid = animatedGeometries.elements.size() + animatedGeometries.pendingElements++;
			currentCmdBuffer->emplace_back(
				[&, uid]() { animatedGeometries.elements[uid] = new SceneAnimatedGeometry(); });
		}

		return uid;
	}


	template<CSceneElem T>
	void EnqueueDestroyCmd(size_t uid)
	{
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			currentCmdBuffer->emplace_back([&, uid]() {
				auto elem = static_cast<T*>(geometries.elements[uid]);
				geometries.elements[uid] = nullptr;
				// TODO: deferred deleting of scene objects
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			currentCmdBuffer->emplace_back([&, uid]() {
				auto elem = static_cast<T*>(cameras.elements[uid]);
				cameras.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			currentCmdBuffer->emplace_back([&, uid]() {
				auto elem = static_cast<T*>(spotlights.elements[uid]);
				spotlights.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<ScenePointlight, T>) {
			currentCmdBuffer->emplace_back([&, uid]() {
				auto elem = static_cast<T*>(pointlights.elements[uid]);
				pointlights.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneDirlight, T>) {
			currentCmdBuffer->emplace_back([&, uid]() {
				auto elem = static_cast<T*>(directionalLights.elements[uid]);
				directionalLights.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneReflProbe, T>) {
			currentCmdBuffer->emplace_back([&, uid]() {
				auto elem = static_cast<T*>(reflProbs.elements[uid]);
				reflProbs.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneAnimatedGeometry, T>) {
			currentCmdBuffer->emplace_back([&, uid]() {
				auto elem = static_cast<T*>(animatedGeometries.elements[uid]);
				animatedGeometries.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
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
